# mini-pgw


сборка
```bash
cd ~/mini-pgw
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Запуск клиента 
```bash
mini-pgw/build/src/client/pgw_client 001010123456789
```

Запуск сервера
```bash
mini-pgw/build/src/server/pgw_server
```
