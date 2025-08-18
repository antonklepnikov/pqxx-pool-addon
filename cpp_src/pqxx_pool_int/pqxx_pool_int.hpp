#ifndef PQXX_POOL_INT_HPP_AK
#define PQXX_POOL_INT_HPP_AK


#include <pqxx/pqxx>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <utility>
#include <format>
#include <unordered_set>
#include <string>
#include <string_view>
#include <optional>
#include <stdexcept>


namespace pqxxplint {
  

  struct ConnectionParams {
    std::string dbName{};
    std::string user{};
    std::string password{};
    std::string hostAddr{"127.0.0.1"};
    std::string port{"5432"};
    int poolSize{10};
  }; 


  class BasicConnection;
  class BasicTransaction;
  

  using ConnectionPtr = std::unique_ptr<pqxx::connection>;

  class ConnectionManager {
    friend class BasicConnection;
    private:
      std::unordered_set<std::string> prepares{};
      std::mutex preparesMutex{};
      ConnectionPtr connection{};
    public:
      ConnectionManager(ConnectionPtr& connection): connection(std::move(connection)){};
      ConnectionManager(const ConnectionManager&) = delete;
      ConnectionManager& operator=(const ConnectionManager&) = delete;
      void prepare(const std::string& name, const std::string& def) {
        std::scoped_lock lock(preparesMutex);
        if (prepares.count(name)) {
          return;
        }
        connection->prepare(name, def);
        prepares.insert(name);
      }
      bool isOpen() {
        return connection->is_open();
      }
      void close() {
        connection->close();
      }
  };

  
  using ManagerPtr = std::unique_ptr<ConnectionManager>;
  
  class ConnectionPool {
    private:
      std::mutex connectionsMutex{};
      std::condition_variable connectionsCond{};
      std::queue<ManagerPtr> connections{};
      std::string statusMsg{"Error: there are no connections in the pool"};
    public:
      ConnectionPool(const ConnectionParams& params) {
        int poolSize = params.poolSize;
        for (int i = 0; i < poolSize; ++i) {
          const std::string paramsString = {
            "dbname = " + params.dbName +
            " user = " + params.user +
            " password = " + params.password +
            " hostaddr = " + params.hostAddr +
            " port = " + params.port
          };
          ConnectionPtr connection = std::make_unique<pqxx::connection>(paramsString);
          ManagerPtr manager = std::make_unique<ConnectionManager>(connection);
          connections.push(std::move(manager));
        }
        statusMsg = "SQL: " + 
          std::to_string(poolSize) + " connections have been established to database " + 
          "\"" + params.hostAddr + ":" + params.port + "\\" + params.dbName + "\"" + '\n';
      }
      ManagerPtr borrowConnection() {
        std::unique_lock lock(connectionsMutex);
        connectionsCond.wait(lock, [this]() { return !connections.empty(); });
        ManagerPtr manager = std::move(connections.front());
        connections.pop();
        return manager;
      }
      void returnConnection(ManagerPtr& manager) {
        {
          std::scoped_lock lock(connectionsMutex);
          connections.push(std::move(manager));
        }
        connectionsCond.notify_one();
      }
      std::string getPoolStatus() { return statusMsg; }
  };

  
  class BasicConnection final {
    private:
      ConnectionPool& pool;
      ManagerPtr manager;
    public:
      BasicConnection(ConnectionPool& pool): pool(pool) {
        manager = pool.borrowConnection();
      }
      ~BasicConnection() {
        pool.returnConnection(manager);
      }
      BasicConnection(const BasicConnection&) = delete;
      BasicConnection& operator=(const BasicConnection&) = delete;
      pqxx::connection& get() const { return *manager->connection; }
      operator pqxx::connection&() { return get(); }
      operator const pqxx::connection&() const { return get(); }
      pqxx::connection* operator->() { return manager->connection.get(); }
      const pqxx::connection* operator->() const { return manager->connection.get(); }
      void prepare(std::string_view name, std::string_view def) {
        manager->prepare(std::string(name), std::string(def));
      }
  };

  
  class Query {
    private:
      std::string queryString;
      pqxx::work& transactionView;
      std::string convertResultToJsonString(pqxx::result in) {
        const int columns = in.columns();
        const int rows = in.size();
        std::string out{"["};
        for (auto const &row: in) {
          const int rowNum = row.num() + 1;
          out += "{\"";
          for (auto const &field: row) {
            const int columnNum = field.num() + 1;
              out += field.name();
              out += "\":\"";
              out += field.c_str();
              out += columns - columnNum == 0 ? "" : "\",\"";
          }
          out += rows - rowNum == 0 ? "\"}" : "\"},";
        }
        return out += ']';
      }
    public:
      Query(std::string_view str, pqxx::work& txView)
        : queryString(str), transactionView(txView) {}
      const char* data() const {
        return queryString.data();
      }
      operator std::string() const {
        return { queryString.begin(), queryString.end() };
      }
      operator std::string_view() const {
        return { queryString.data(), queryString.size() };
      }
      std::string exec(const std::vector<std::string>& params) {
        pqxx::params execParams{};
        if (!params.empty()) {
          // std::vector<std::string> tokens = splitParams(params);
          for (const std::string& p: params) {
            execParams.append(p);
          }
        }
        pqxx::result res = transactionView.exec_params(queryString, execParams);
        return convertResultToJsonString(res);
      }
      std::string operator()(const std::vector<std::string>& params) {
        return exec(params);
      }
  };


  class BasicTransaction {
    friend class Query;
    friend class ConnectionPool;
    private:
      BasicConnection connection;
      pqxx::work transaction;
    public:
      BasicTransaction(ConnectionPool& pl)
        : connection(pl), transaction(connection.get()) {}
      BasicTransaction(const BasicTransaction&) = delete;
      BasicTransaction& operator=(const BasicTransaction&) = delete;
      pqxx::work& get() { return transaction; }
      operator pqxx::work&() { return get(); }
      std::string query(std::string query, const std::vector<std::string>& params) {
        std::string res{};
        try {
          pqxxplint::Query q(query, this->get());
          res = q(params);
        } catch (std::exception& err) {
        throw;
        }
        return res;
      }
      void commit() { transaction.commit(); }
  };

  
} // namespace pqxxplint

#endif
