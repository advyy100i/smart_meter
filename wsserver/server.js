const ws = require("ws");
const { Kafka } = require("kafkajs");

const kafka = new Kafka({
  clientId: "websocket-app",
  brokers: ["localhost:9092"],
});
const consumer = kafka.consumer({ groupId: "websocket-group" });

const wss = new ws.Server({ port: 8080 });
const clients = new Map();

wss.on("connection", function (ws) {
  ws.on("message", (message) => {
    const { meterId } = JSON.parse(message);
    clients.set(ws, Number(meterId));
    console.log(`Client subscribed to meter ID: ${meterId}`);
  });

  ws.on("close", () => {
    console.log("Client disconnected");
    clients.delete(ws);
  });

  ws.on("error", (error) => {
    console.error("WebSocket error:", error);
    clients.delete(ws);
  });
});

async function main() {
  await consumer.connect();
  await consumer.subscribe({ topic: "meter-readings", fromBeginning: true });
  await consumer.run({
    eachMessage: async ({ topic, partition, message }) => {
      if (topic === "meter-readings") {
        console.log("Received message", message.value.toString());
        const meterReading = JSON.parse(message.value.toString());
        clients.forEach((meterId, ws) => {
          if (meterReading.meterId === meterId) {
            try {
              ws.send(JSON.stringify(meterReading));
            } catch (error) {
              console.error('Failed to send message to WebSocket client:', error);
            }
          }
        });
      }
    },
  });
}

main().catch(console.error);

process.on("SIGINT", async function () {
  console.log("Disconnecting data sink");
  await consumer.disconnect();
  process.exit();
});