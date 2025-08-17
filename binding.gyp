{
  "targets": [{
      "target_name": "pqxx-pool-addon",
      "cflags!": [ "-fno-exceptions", "-fno-rtti" ],
      "cflags_cc!": [ "-fno-exceptions", "-fno-rtti" ],
      "sources": [
        "cpp_src/main.cpp",
        "cpp_src/connection_pool.cpp",
        "cpp_src/basic_transaction.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "libraries": [
        "-lpq",
        "-lpqxx"
      ],
      "dependecies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [ 
        "NAPI_CPP_EXCEPTIONS", 
      ]    
  }]
}