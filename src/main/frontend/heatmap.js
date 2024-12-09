export class Heatmap {
    constructor(imageId, canvasId) {
        this.image = document.getElementById(imageId);
        this.canvas = document.getElementById(canvasId);
        this.context = this.canvas.getContext('2d');
        this.heatmapData = [];

        this.image.onload = () => {
            this.adjustCanvasSize();
        };

        window.addEventListener('resize', () => {
            this.adjustCanvasSize();
        });

        // If the image is already loaded (cached), trigger the onload event
        if (this.image.complete) {
            this.adjustCanvasSize();
        }
    }

    adjustCanvasSize() {
        try {
            const imageWidth = this.image.clientWidth;
            const imageHeight = this.image.clientHeight;

            if (imageWidth > 0 && imageHeight > 0) {
                this.canvas.width = imageWidth;
                this.canvas.height = imageHeight;
                this.drawHeatmap();
            } else {
                console.warn("Image dimensions are not valid for adjusting the canvas size.");
            }
        } catch (error) {
            console.error("Error adjusting canvas size: ", error);
        }
    }

    drawHeatmap() {
        try {
            const ctx = this.context;
            const width = this.canvas.width;
            const height = this.canvas.height;

            ctx.clearRect(0, 0, width, height); // Clear the canvas

            this.heatmapData.forEach(point => {
                if (point.x >= 0 && point.y >= 0 && point.x <= width && point.y <= height) {
                    this.drawPoint(point.x, point.y, point.value);
                }
            });

            this.applyHeatmapGradient();
        } catch (error) {
            console.error("Error drawing heatmap: ", error);
        }
    }

    drawPoint(x, y, intensity) {
        try {
            const radius = 50;
            const gradient = this.context.createRadialGradient(x, y, 0, x, y, radius);
            gradient.addColorStop(0, `rgba(255, 0, 0, ${intensity})`);
            gradient.addColorStop(1, 'rgba(255, 0, 0, 0)');

            this.context.beginPath();
            this.context.arc(x, y, radius, 0, Math.PI * 2);
            this.context.fillStyle = gradient;
            this.context.fill();
        } catch (error) {
            console.error("Error drawing point: ", error);
        }
    }

    applyHeatmapGradient() {
        try {
            const ctx = this.context;
            const width = this.canvas.width;
            const height = this.canvas.height;

            if (width > 0 && height > 0) {
                const imageData = ctx.getImageData(0, 0, width, height);
                const data = imageData.data;

                for (let i = 0; i < data.length; i += 4) {
                    const alpha = data[i + 3] / 255;
                    data[i] = 255;
                    data[i + 1] = (1 - alpha) * 255;
                    data[i + 2] = 0;
                    data[i + 3] = alpha * 255;
                }

                ctx.putImageData(imageData, 0, 0);
            } else {
                console.warn("Canvas dimensions are not valid for applying heatmap gradient.");
            }
        } catch (error) {
            console.error("Error applying heatmap gradient: ", error);
        }
    }

    setHeatmapData(newData) {
        this.heatmapData = newData;
        this.drawHeatmap();
    }

    addHeatmapPoint(point) {
        this.heatmapData.push(point);
        this.drawHeatmap();
    }
}

export default Heatmap;
