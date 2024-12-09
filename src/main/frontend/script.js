const SOCKET_URL = 'ws://localhost:8765';

const floorplan = document.getElementById('floorplan');
const positionsContainer = document.getElementById('positions-container');
const heatmapContainer = document.getElementById('heatmap-container')
const sidebarContent = document.getElementById('sidebar-content');
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

function onWebSocketOpen() {
    console.log('WebSocket connection established.');
}

function onWebSocketMessage(event) {
    try {
        const position = JSON.parse(event.data);
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
        // updatePositionHeatmap(positions[position.id])
    } else {
        createPositionLabel(position);
    }
}

function createPositionLabel(position) {
    const label = document.createElement('div');
    label.classList.add('position-label');
    label.title = position.id;
    positionsContainer.appendChild(label);
    positions[position.id] = { element: label, data: position };
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
    const img = floorplan;
    if (!img.naturalWidth || !img.naturalHeight) {
        img.onload = () => updatePositionLabel(posObj);
        return;
    }

    if (!posObj || !posObj.data) {
        console.error("Position data is missing", posObj);
        return;
    }

    const xRatio = img.width / img.naturalWidth;
    const yRatio = img.height / img.naturalHeight;
    const { x, y } = posObj.data;

    posObj.element.style.position = 'absolute';
    posObj.element.style.left = `${x * xRatio}px`;
    posObj.element.style.top = `${y * yRatio}px`;
}

function updateSidebar() {
    positionList.innerHTML = '';
    for (const [id, lastSeen] of Object.entries(sidebarData)) {
        const content = document.createElement('li');
        content.innerText = `${id} | last seen: ${new Date(lastSeen).toLocaleString()}`;
        positionList.appendChild(content);
    }

}

