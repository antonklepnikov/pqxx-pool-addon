import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";
import path from "node:path";

const require = createRequire(import.meta.url);
const __dirname = path.dirname(fileURLToPath(import.meta.url));

const addonPath = path.join(
  __dirname,
  "build",
  "Release",
  "pqxx-pool-addon.node"
);

const poolAddon = require(addonPath);

const connectionParams = {
  dbName: "pqxxpl_test",
  user: "pqxxpl_test",
  password: "123654",
};

const pool = new poolAddon.ConnectionPool(connectionParams);
console.log(pool.getPoolStatus());

//export const createPool = () => new poolAddon.ConnectionPool(connectionParams);
export const getTx = () => new poolAddon.BasicTransaction(pool);
