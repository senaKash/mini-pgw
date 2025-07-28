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


У меня в конце появилась вот эта ошибка и я не могу проверить работает ли норвая реализация main у клиента(должно если mini-pgw/build/src/client/pgw_client 001010123456789 то только 1 сессия а если mini-pgw/build/src/client/pgw_client то из конфига берется количество клиентов, которых нужно запустить)
<img width="1280" height="681" alt="image" src="https://github.com/user-attachments/assets/0206b589-8315-4dda-b8ff-5acbd1ae8be9" />

