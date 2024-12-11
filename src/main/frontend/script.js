import Heatmap from "./heatmap.js";

const SOCKET_URL = 'ws://localhost:8765';
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
const socket = new WebSocket(SOCKET_URL);

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
        const dto = JSON.parse(event.data);
        console.log(dto);
        if (dto.positions) {
            dto.positions.forEach(position => {
                console.log("Position:", position);
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
        id: id, x: x / width, y: y / height, scale: (img.width / width) * 2 * uncertainty
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
        updatePositionData(position);
        Object.values(positions).forEach(updatePositionLabel)
        updatePositionHeatmap(positions[position.id])
    } else {
        createPositionLabel(position);
    }
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

    posObj.element.innerHTML = posObj.data.id;
    posObj.element.style.position = 'absolute';
    posObj.element.style.left = `${x * img.width}px`;
    posObj.element.style.top = `${y * img.height}px`;
    posObj.element.style.width = `${scale}px`;
    posObj.element.style.height = `${scale}px`;
}

function updatePositionHeatmap(posObj) {
    const {x, y} = posObj.data;
    const newPoint = {x: x * canvas.width, y: y * canvas.height, value: 0.3};
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

