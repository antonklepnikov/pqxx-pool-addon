const pqxxPoolAddon = require("./build/Release/pqxx-pool-addon.node");

const connectionParams = {
  dbName: "pqxxpl_test",
  user: "pqxxpl_test",
  password: "123654",
};

try {
  const pool = new pqxxPoolAddon.ConnectionPool(connectionParams);

  const tableTransaction = new pqxxPoolAddon.BasicTransaction(pool);
  tableTransaction.query(`DROP TABLE IF EXISTS test_users`);
  tableTransaction.query(`
    CREATE TABLE IF NOT EXISTS test_users (
      user_id SERIAL PRIMARY KEY,
      username TEXT UNIQUE,
      role TEXT
    )
  `);
  tableTransaction.commit();

  const createUserTransaction = new pqxxPoolAddon.BasicTransaction(pool);
  const createUserParamsQuery = `
    INSERT INTO test_users (username, role) VALUES ($1, $2) RETURNING user_id`;
  const userRes1 = createUserTransaction.query(
    createUserParamsQuery,
    "Antonk-1,Admin"
  );
  console.log(`Created user with id: ${userRes1}`);
  const userRes2 = createUserTransaction.query(
    createUserParamsQuery,
    "Antonk-2,Admin"
  );
  console.log(`Created user with id: ${userRes2}`);
  const userRes3 = createUserTransaction.query(
    `INSERT INTO test_users (username) VALUES ('Antonk 3') RETURNING user_id`
  );
  console.log(`Created user with id: ${userRes3}`);
  createUserTransaction.commit();

  const readUsersTransaction = new pqxxPoolAddon.BasicTransaction(pool);
  const usersJsonString = readUsersTransaction.query(
    `SELECT * FROM test_users ORDER BY user_id ASC`
  );
  console.log(JSON.parse(usersJsonString));
} catch (err) {
  console.error(`SQL pool error: ${err.message}`);
}

module.exports = pqxxPoolAddon;
