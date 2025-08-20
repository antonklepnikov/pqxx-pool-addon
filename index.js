import { getTx } from "./pool.js";

const tableTx = getTx();
tableTx.query(`DROP TABLE IF EXISTS test_users`);
tableTx.query(`
  CREATE TABLE IF NOT EXISTS test_users (
    user_id SERIAL PRIMARY KEY,
    username TEXT UNIQUE,
    role TEXT,
    is_admin BOOL
  )
`);
tableTx.commit();

const userTx = getTx();
const userParamsQuery = `
  INSERT INTO test_users (username, is_admin) VALUES ($1, $2) RETURNING user_id`;
const userRes1 = userTx.query(userParamsQuery, ["Antonk-1", true]);
console.log(`Created user with id: ${userRes1[0].user_id}`);
userTx.commit();

const readUsersTx = getTx();
const users = readUsersTx.query(
  `SELECT * FROM test_users ORDER BY user_id ASC`
);
console.log(users);

// const pqxxPoolAddon = require("./build/Release/pqxx-pool-addon.node");

// const connectionParams = {
//   dbName: "pqxxpl_test",
//   user: "pqxxpl_test",
//   password: "123654",
// };

// try {
//   // const pool = new pqxxPoolAddon.ConnectionPool(connectionParams);
//   // console.log(pool.getPoolStatus());

//   const tableTransaction = getTx();
//   tableTransaction.query(`DROP TABLE IF EXISTS test_users`);
//   tableTransaction.query(`
//     CREATE TABLE IF NOT EXISTS test_users (
//       user_id SERIAL PRIMARY KEY,
//       username TEXT UNIQUE,
//       role TEXT
//     )
//   `);
//   tableTransaction.commit();

//   const createUserTransaction = getTx();
//   const createUserParamsQuery = `
//     INSERT INTO test_users (username, role) VALUES ($1, $2) RETURNING user_id`;
//   const userRes1 = createUserTransaction.query(createUserParamsQuery, [
//     "Antonk-1",
//     "Admin",
//   ]);
//   console.log(`Created user with id: ${userRes1}`);
//   const userRes2 = createUserTransaction.query(createUserParamsQuery, [
//     "Antonk-2",
//     "User",
//   ]);
//   console.log(`Created user with id: ${userRes2}`);
//   const userRes3 = createUserTransaction.query(
//     `INSERT INTO test_users (username) VALUES ('Antonk 3') RETURNING user_id`
//   );
//   console.log(`Created user with id: ${userRes3}`);
//   createUserTransaction.commit();

//   const readUsersTransaction = getTx();
//   const users = readUsersTransaction.query(
//     `SELECT * FROM test_users ORDER BY user_id ASC`
//   );
//   console.log(users);
//   // const json = users.json();
//   // console.log(json);
// } catch (err) {
//   console.error(`SQL pool error: ${err.message}`);
// }

// module.exports = pqxxPoolAddon;
