const net = require("net");
const { Kafka } = require("kafkajs");

const KAFKA_BROKERS = process.env.KAFKA_BROKERS || "localhost:9092";
const KAFKA_TOPIC = process.env.KAFKA_TOPIC || "meter-readings";
const KAFKA_CLIENT_ID = process.env.KAFKA_CLIENT_ID || "tcp-to-kafka";

const kafka = new Kafka({
  clientId: KAFKA_CLIENT_ID,
  brokers: KAFKA_BROKERS,
});
const producer = kafka.producer();
const server = net.createServer();

function parseTCPPacket(buffer) {
  if (buffer.length < 16) {
    throw new Error("Invalid packet length");
  }

  const timestamp = buffer.readUInt32LE(0);
  const meterId = buffer.readUInt32LE(4);
  const voltage = (buffer.readUInt16LE(8) / 100).toFixed(2);
  const current = (buffer.readUInt16LE(10) / 100).toFixed(2);
  const power = (buffer.readUInt32LE(12) / 100).toFixed(2);
  const energy = (buffer.readUInt32LE(16) / 100).toFixed(2);

  return {
    timestamp,
    meterId,
    voltage,
    current,
    power,
    energy,
  };
}

async function main() {
  await producer.connect();
  console.log("Kafka producer connected");

  server.on("connect", async function (socket) {
    console.log(
      `Client connected: ${socket.remoteAddress}:${socket.remotePort}`
    );

    socket.keepAlive = true;
    socket.setEncoding("binary");

    socket.on("data", async (data) => {
      try {
        const rawBuffer = Buffer.from(data, "binary");
        const parsedData = parseTCPPacket(rawBuffer);
        await producer.send({
          topic: KAFKA_TOPIC,
          messages: [{ value: JSON.stringify(parsedData) }],
        });
      } catch (error) {
        console.error("Error processing data:", error);
      }
    });

    socket.on("end", function () {
      console.log(
        `Client disconnected: ${socket.remoteAddress}:${socket.remotePort}`
      );
    });
  });

  server.listen(4000, () => {
    console.log("TCP Server listening on port 4000");
  });
}

main().catch(console.error);

process.on("SIGINT", async () => {
  console.log("Shutting down TCP server");
  server.close();
  await producer.disconnect();
  process.exit(0);
});
