import Heatmap from "./heatmap.js";

const SOCKET_URL = 'ws://localhost:8765';
const heatmap = new Heatmap('floorplan', 'heatmapCanvas')
const floorplan = document.getElementById('floorplan');
const img = floorplan;
const xRatio = img.width / img.naturalWidth;
const yRatio = img.height / img.naturalHeight;
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

function onWebSocketOpen() {
    console.log('WebSocket connection established.');
}

function onWebSocketMessage(event) {
    try {
        const position = JSON.parse(event.data); // TODO: update DTO and add relative position calculation
        console.log("Position:", position);
        handlePosition(position);
    } catch (err) {
        console.error('Error parsing WebSocket message:', err);
        console.log("WebSocket message:", event);
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
        updatePositionLabel(positions[position.id]);
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

function removePosition(id) {
    const pos = positions[id];
    if (pos) {
        positionsContainer.removeChild(pos.element);
        delete positions[id];
    }
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

    const {x, y} = posObj.data;

    posObj.element.innerHTML = posObj.data.id;
    posObj.element.style.position = 'absolute';
    posObj.element.style.left = `${x * xRatio}px`;
    posObj.element.style.top = `${y * yRatio}px`;
}

function updatePositionHeatmap(posObj) {
    const {x, y} = posObj.data;
    const newPoint = {x: x * xRatio, y: y * yRatio, value: 0.3};
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

