gcc -std=c++11 \
-lboost_system \
-lboost_filesystem \
-lboost_regex \
-lboost_serialization \
-lpthread \
-lstdc++ \
-o client \
Client.cpp \

gcc -std=c++11 \
-lboost_system \
-lboost_regex \
-lboost_serialization \
-lpthread \
-lstdc++ \
-o threadpool_server \
threadpool_server.cpp hash.cpp 