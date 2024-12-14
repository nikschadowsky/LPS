import Heatmap from "./heatmap.js";

//const host = window.location.hostname;
const host = window.location.hostname;
const SOCKET_URL = `ws://${host}:8080/api/ws`;
const heatmap = new Heatmap('floorplan', 'heatmapCanvas')
const canvas = document.getElementById('heatmapCanvas');
const floorplan = document.getElementById('floorplan');
const img = floorplan;
const positionsContainer = document.getElementById('positions-container');
const positionList = document.getElementById('position-list')
const tabButtons = document.querySelectorAll('.tab-button');
const tabContents = document.querySelectorAll('.tab-content');

const positions = {};
const sidebarData = [];
const positionBuffer = {};
const socket = new WebSocket(SOCKET_URL);

document.addEventListener("DOMContentLoaded", () => {
    tabButtons.forEach(button => button.classList.remove('active'));
    tabContents.forEach(content => content.classList.remove('active'));

    tabButtons[1].classList.add('active');
    document.getElementById(`${tabButtons[1].dataset.tab}-container`).classList.add('active');
})
socket.addEventListener('open', onWebSocketOpen);
socket.addEventListener('message', onWebSocketMessage);
socket.addEventListener('close', onWebSocketClose);
window.addEventListener('resize', onWindowResize);
tabButtons.forEach(button => button.addEventListener('click', onTabButtonClick));
tabButtons[1].addEventListener('click', () => heatmap.setHeatmapData([]));
tabButtons[0].addEventListener('click', () => Object.values(positions).forEach(updatePositionLabel));

function onWebSocketOpen() {
    console.log('WebSocket connection established.');
}

function onWebSocketMessage(event) {
    try {
        let dto = event.data.replace(/Infinity/g, `\"Infinity\"`);
        console.log(dto);
        dto = JSON.parse(dto, (key, value) => {
            if (value === "Infinity") {
                return Infinity;
            }
            return value;
        });
        console.log(dto);
        if (dto.positions) {
            dto.positions.forEach(position => {
                console.log("Position:", position);
                if (position.x === Infinity || position.y === Infinity) {
                    return;
                }
                const relativePosition = calculateRelativePosition(position, dto.room);
                console.log("Relative Position:", relativePosition)
                handlePosition(relativePosition);
            })
        }
    } catch (err) {
        console.error('Error parsing WebSocket message:', err);
        console.log("WebSocket message:", event);
    }

}

function calculateRelativePosition(position, room) {
    const {id, x, y, uncertainty} = position;
    const {distanceAB: width, distanceAD: height} = room;
    return {
        id: id, x: x / width, y: y / height, scale: (img.width / width) * uncertainty
    }
}

function onWebSocketClose() {
    console.log('WebSocket connection closed.');
}

function onWindowResize() {
    Object.values(positions).forEach(updatePositionLabel);
}

function onTabButtonClick(event) {
    tabButtons.forEach(btn => btn.classList.remove('active'));
    event.currentTarget.classList.add('active');

    tabContents.forEach(content => content.classList.remove('active'));
    document.getElementById(`${event.currentTarget.dataset.tab}-container`).classList.add('active');
}

function handlePosition(position) {
    if (positions[position.id]) {
        const smoothedPosition = smoothPositionWithAverage(position)
        //const smoothedPosition = smoothPositionWithMedian(position)
        updatePositionData(smoothedPosition);
        Object.values(positions).forEach(updatePositionLabel)
        updatePositionHeatmap(positions[position.id])
    } else {
        createPositionLabel(position);
        positionBuffer[position.id] = [{...position}];
    }
}

function smoothPositionWithAverage(position) {
    positionBuffer[position.id].push({...position});
    if (positionBuffer[position.id].length > 3) {
        positionBuffer[position.id].shift();
    }
    console.log("Position:", position);
    console.log("Buffer:", positionBuffer[position.id]);
    const smoothedPosition = positionBuffer[position.id].reduce((acc, pos) => {
        acc.x += pos.x;
        acc.y += pos.y;
        acc.scale += pos.scale;
        return acc;
    }, {x: 0, y: 0, scale: 0});
    smoothedPosition.x /= positionBuffer[position.id].length;
    smoothedPosition.y /= positionBuffer[position.id].length;
    const sortedScale = positionBuffer[position.id].map(p => p.scale).sort((a, b) => a - b);
    smoothedPosition.scale = sortedScale[Math.floor(sortedScale.length / 2)];
    smoothedPosition.id = position.id;
    console.log("Smoothed Position:", smoothedPosition);
    return smoothedPosition;
}

function smoothPositionWithMedian(position) {
    positionBuffer[position.id].push({...position});
    if (positionBuffer[position.id].length > 3) {
        positionBuffer[position.id].shift();
    }
    console.log("Position:", position);
    console.log("Buffer:", positionBuffer[position.id]);
    const sortedX = positionBuffer[position.id].map(p => p.x).sort((a, b) => a - b);
    const sortedY = positionBuffer[position.id].map(p => p.y).sort((a, b) => a - b);
    const sortedScale = positionBuffer[position.id].map(p => p.scale).sort((a, b) => a - b);

    const medianX = sortedX[Math.floor(sortedX.length / 2)];
    const medianY = sortedY[Math.floor(sortedY.length / 2)];
    const medianScale = sortedScale[Math.floor(sortedScale.length / 2)];
    // TODO decide between min or median scale
    return {id: position.id, x: medianX, y: medianY, scale: medianScale};
}

function createPositionLabel(position) {
    const label = document.createElement('div');
    label.classList.add('position-label');
    label.title = position.id;
    positionsContainer.appendChild(label);
    positions[position.id] = {element: label, data: position};
    updatePositionLabel(positions[position.id]);
}

function updatePositionData(position) {
    const pos = positions[position.id];
    if (pos) pos.data = position;
    sidebarData[position.id] = Date.now();
    updateSidebar(position);
}

function updatePositionLabel(posObj) {
    if (!img.naturalWidth || !img.naturalHeight) {
        img.onload = () => updatePositionLabel(posObj);
        return;
    }

    if (!posObj || !posObj.data) {
        console.error("Position data is missing", posObj);
        return;
    }

    const {x, y, scale} = posObj.data;

    console.log(`device: ${posObj.data.id}, timestamp: ${Date.now() - sidebarData[posObj.data.id]}`);
    const timestamp = Date.now() - sidebarData[posObj.data.id];
    if (timestamp < 5000 || isNaN(timestamp)) {
        posObj.element.style.backgroundColor = 'rgba(0, 128, 0, 0.5)';
    } else if (timestamp < 300000) {
        posObj.element.style.backgroundColor = 'rgba(255,213,52,0.5)';
    } else {
        posObj.element.style.backgroundColor = 'rgba(206,25,25,0.5)';
    }

    posObj.element.innerHTML = `<b>${posObj.data.id}</b>`;
    posObj.element.style.position = 'absolute';
    posObj.element.style.left = `${x * img.width}px`;
    posObj.element.style.top = `${y * img.height}px`;
    posObj.element.style.width = `${scale}px`;
    posObj.element.style.height = `${scale}px`;
}

function updatePositionHeatmap(posObj) {
    const {x, y} = posObj.data;
    const newPoint = {x: x * canvas.width, y: y * canvas.height, value: 0.05};
    heatmap.addHeatmapPoint(newPoint);
}

function updateSidebar() {
    positionList.innerHTML = '';
    for (const [id, lastSeen] of Object.entries(sidebarData)) {
        const content = document.createElement('li');
        content.innerHTML = `
    <span class="user-id">${id}</span>
    <span class="last-seen"> | Last seen: ${new Date(lastSeen).toLocaleString()}</span>
`;
        positionList.appendChild(content);
    }

}

