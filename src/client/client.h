#ifndef PGW_CLIENT_H
#define PGW_CLIENT_H

#include <string>
#include <vector>
#include <cstdint>

// Прототипы функций клиента
int run_client(const std::string& imsi, const std::string& server_ip, uint16_t server_port);
int extend_session(const std::string& imsi, const std::string& server_ip, uint16_t server_port);
void run_multiple_clients(const std::string& server_ip, uint16_t server_port, int num_clients);
std::vector<uint8_t> imsi_to_bcd(const std::string& imsi);

// Объявление функции генерации IMSI
std::string generate_imsi();

#endif  // PGW_CLIENT_H
