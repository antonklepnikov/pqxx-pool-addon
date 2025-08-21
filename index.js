import { getTx } from "./pqxx_pool.js";

try {
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
} catch (err) {
  console.error(`SQL pool error: ${err.message}`);
}

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
