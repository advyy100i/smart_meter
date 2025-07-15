const { Kafka } = require("kafkajs");
const { MongoClient } = require("mongodb");

const KAFKA_GROUP_ID = process.env.KAFKA_GROUP_ID || "datasink-group";
const KAFKA_HOST = process.env.KAFKA_BROKERS || "localhost";
const KAFKA_PORT = process.env.KAFKA_PORT || "9092";
const KAFKA_TOPIC = process.env.KAFKA_TOPIC || "meter-readings";

const MONGO_HOST = process.env.MONGO_HOST || "127.0.0.1";
const MONGO_PORT = process.env.MONGO_PORT || "27017";
const MONGO_DB_NAME = process.env.MONGO_DB_NAME || "meter-readings";
const MONGO_COLLECTION_NAME = process.env.MONGO_COLLECTION_NAME || "readings";

const kafka = new Kafka({
  clientId: "data-sink-app",
  brokers: [`${KAFKA_HOST}:${KAFKA_PORT}`],
});

const consumer = kafka.consumer({ groupId: `${KAFKA_GROUP_ID}` });

let db;

async function connectToDB() {
  try {
    const client = await MongoClient.connect(
      `mongodb://${MONGO_HOST}:${MONGO_PORT}/`
    );
    db = client.db(MONGO_DB_NAME);
    console.log("Connected to the database");
  } catch (error) {
    console.error("Error connecting to the database", error);
    throw error;
  }
}

async function main() {
  await connectToDB();
  console.log("Reached");

  await consumer.connect();
  console.log("Connected to Kafka");

  // subscribe to our readings topic
  await consumer.subscribe({ topic: KAFKA_TOPIC, fromBeginning: true });

  const collection = db.collection(MONGO_COLLECTION_NAME);
  await consumer.run({
    eachMessage: async ({ topic, _, message }) => {
      if (topic === KAFKA_TOPIC) {
        const data = JSON.parse(message.value.toString());

        // convert the epoch time to normal time
        var date = new Date(0);
        date.setUTCSeconds(data.timestamp);
        data.timestamp = date;

        try {
          collection.insertOne(data);
          console.log(data);
        } catch (e) {
          console.error(e);
          throw e;
        }
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
