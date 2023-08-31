#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment (lib, "ws2_32.lib")

class SocketClient
{
private:
	int portNumber;
	//char ipAddress [20];
	std::string ipAddress;
	WSADATA wsaData;
	SOCKET client;
	sockaddr_in serverAddr;
	static const int bufferSize = 1024;
	char buffer[bufferSize];

public:
	explicit SocketClient(const int& _portNumber, const std::string& _ipAddress) :
		portNumber(_portNumber),
		ipAddress(_ipAddress)
	{}
	~SocketClient()
	{
		std::cout << "Socket Closing and Cleanup.\n";
		closesocket(client);
		WSACleanup();
	}

	//Creating Socket
	void creatSocket()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			throw std::runtime_error("Winsock did not startup\n");
		}

		client = socket(AF_INET, SOCK_STREAM, 0);
		if (client == INVALID_SOCKET)
		{
			WSACleanup();
			throw std::runtime_error("Socket Creation Failed\n");
		}

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
		serverAddr.sin_port = htons(portNumber);

		if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			closesocket(client);
			WSACleanup();
			throw std::runtime_error("Faile to connect to server\n");
		}
		std::cout << "Client Connected to server.\n";
	}

	[[nodiscard]] std::string receiveMessage()
	{
		int bytesReceived;
		bytesReceived = recv(client, buffer, bufferSize, 0);
		if (bytesReceived <= 0)
			throw std::runtime_error("Server Disconnected!!!\n");
		std::string sendMessage = "OK";
		if (send(client, sendMessage.c_str(), sendMessage.length() + 1, 0) == SOCKET_ERROR)
		{
			throw std::runtime_error("Fail to send acknoledgement\n");
		}
		else
			return buffer;
		
	}

	void sendMessage(const std::string& message)
	{
		if (send(client, message.c_str(), message.length()+1, 0) == SOCKET_ERROR)
		{
			throw std::runtime_error("Fail to send Data\n");
		}
		std::cout << "Message sent\n";
	}

};

inline int logIn()
{
	int action;
	std::cout << "\n\t\tLog IN as:\n\n"
		<< "\t1 -> Admin\n"
		<< "\t2 -> Sales Rep\n"
		<< "\t3 -> Exit\n";
	std::cout << "Action: ";
	std::cin >> action;
	return action;
}

inline int promptAdminLogin()
{
	int action;
	std::cout << "\n\t\tAdmin Login\n"
		<< "\tEnter Password: ";
	std::cin >> action;
	return action;
}
inline std::string promptSalesRepLogin()
{
	std::string userName;
	int password;
	std::cout << "\n\t\tSales Rep Login\n"
		<< "\tEnter user name: ";
	std::cin >> userName;
	std::cout << "\tEnter Password: ";
	std::cin >> password;

	std::ostringstream oss;
	oss << userName << ' ' << password;
	return oss.str();
}
inline int promptAdmin()
{
	int action;
	std::cout << "\n\t\tAdmin Loged In\n"
		<< "\t1 -> Add Product\n"
		<< "\t2 -> Get Product Details\n"
		<< "\t3 -> Sell Product\n"
		<< "\t4 -> Check Out\n"
		<< "\t5 -> Add Sales Rep\n"
		<< "\t6 -> Remove Sales Rep\n"
		<< "\t7 -> Delete Product\n"
		<< "\t8 -> Exit\n";
	std::cout << "Action: ";
	std::cin >> action;
	return action;
}
inline int promptSalesRep()
{
	int action;
	std::cout << "\n\t\tSales Rep\n"
		<< "\t1 -> Sale Product\n"
		<< "\t2 -> Check out\n"
		<< "\t3 -> Exit\n";
	std::cout << "Action: ";
	std::cin >> action;
	return action;
}

int main()
{
	int portNumber;
	std::string ipAddress;
	std::string productName;
	int productQuantity;
	float productPrice = 0;

	std::vector <std::string> cart;
	cart.clear();

	std::cout << "\n\t\tPoint of sale software.\n";
	std::cout << "\nEnter Server Port Number: ";
	std::cin >> portNumber;
	std::cout << "\nEnter Server IP Address: ";
	std::cin >> ipAddress;
	SocketClient client(portNumber, ipAddress);
	try
	{
		client.creatSocket();

		int actionLogin;
		do
		{
			actionLogin = logIn();
			switch (actionLogin)
			{
			case 1:
			{
				int adminPassword;
				bool adminLogedIn = false;
				int x = 0;
				do
				{
					if (x >= 3)
					{
						std::cout << "\nMaximum password trial excedded!!!\n";
						break;
					}
					adminPassword = promptAdminLogin();
					std::ostringstream oss;
					oss << "Admin_login " << "Admin " << adminPassword << ' ' << 0 << ' ' << 0;
					std::string sendMessage;
					sendMessage = oss.str();
					x++;
					try
					{
						client.sendMessage(sendMessage);
						try
						{
							std::string receiveMessage = client.receiveMessage();
							std::cout << receiveMessage;
							if (receiveMessage == "Login_Successful")
							{
								adminLogedIn = true;
								x = 0;
							}
						}
						catch (std::runtime_error& e) { std::cout << e.what(); }

					}
					catch (std::runtime_error& e) { std::cout << e.what(); }
				} while (adminLogedIn == false);

				if (x >= 3)
					break;
				int adminAction;
				do
				{
					adminAction = promptAdmin();
					switch (adminAction)
					{
					case 1:
					{
						std::cout << "Enter product name: ";
						std::cin >> productName;
						std::cout << "Enter Product Price: ";
						std::cin >> productPrice;
						std::cout << "Enter Product Quantity: ";
						std::cin >> productQuantity;
						std::ostringstream oss;
						oss << "Add_product " << productName << ' ' << "null " << productQuantity << ' ' << productPrice;
						std::string sendMessage;
						sendMessage = oss.str();
						try
						{
							client.sendMessage(sendMessage);
							try
							{
								std::string receiveMessage = client.receiveMessage();
								std::cout << std::endl << receiveMessage;
							}
							catch (std::runtime_error& e) { std::cout << e.what(); }
						}
						catch (std::runtime_error& e) { std::cout << e.what(); }
						break;
					}
					case 2:
					{
						std::ostringstream oss;
						oss << "Get_all_product " << "null " << "null " << 0 << ' ' << 0;
						std::string sendMessage;
						sendMessage = oss.str();
						try
						{
							client.sendMessage(sendMessage);
							std::cout << "list of all available products: \n"
								<< "Name\t\tQuantity\t\tPrice";
							std::string command = "";
							while (command != "sent")
							{
								try
								{
									std::string receiveMessage = client.receiveMessage();
									std::istringstream iss(receiveMessage);
									iss >> command >> productName >> productQuantity >> productPrice;
									if (command != "sent")
									{
										std::cout << "\n" << productName << "\t\t" << productQuantity
											<< "\t\t\t" << productPrice;
									}	
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
						}
						catch (std::runtime_error& e) { std::cout << e.what(); }
						
						
						break;
					}
					case 3:
					{
						int productQuantity_s;
						std::cout << "Enter product details: \nEnter product name: ";
						std::cin >> productName;
						std::cout << "Enter product quantity: ";
						std::cin >> productQuantity;
						std::ostringstream oss;
						oss << "Retrieve_product " << productName << ' ' << "null " << productQuantity << ' ' << productPrice;
						std::string sendMessage;
						sendMessage = oss.str();
						try
						{
							client.sendMessage(sendMessage);
							try
							{
								std::string receiveMessage = client.receiveMessage();
								std::istringstream iss(receiveMessage);
								std::string command;
								iss >> command >> productName >> productQuantity_s >> productPrice;
								if (command == "Product_found")
								{
									if (productQuantity_s >= productQuantity)
									{
										std::cout << "Product available and instock.";
										std::ostringstream oss;
										oss << productName << ' ' << productQuantity << ' ' << productPrice << ' ' << productQuantity_s;
										cart.push_back(oss.str());
										std::cout << "\nProduct added to cart.";
									}
									else
										std::cout << "Product out of stock. Available quantity is: " << productQuantity_s << std::endl;
								}
								else
									std::cout << "Product not found!!!";
							}
							catch (std::runtime_error& e) { std::cout << e.what(); }
							
						}
						catch (std::runtime_error& e) { std::cout << e.what(); }
						break;
					}
					case 4:
					{
						std::cout << "Products to pay for:\n"
							<< "Name\t\tQuantity\t\tPrice\t\tAmount\n";
						double totalCost = 0, amount = 0;
						int productQuantity_s, productQuantity_a;
						for (auto& goods : cart)
						{
							std::istringstream iss(goods);
							iss >> productName >> productQuantity >> productPrice >> productQuantity_s;
							amount = productQuantity * productPrice;
							totalCost += amount;
							std::cout << productName << "\t\t" << productQuantity << "\t\t"
								<< productPrice << "\t\t" << amount << "\n";
							productQuantity_a = productQuantity_s - productQuantity;
							std::ostringstream oss;
							oss << "Update_quantity " << productName << " null " << productQuantity_a << ' ' << productPrice;
							std::string sendMessage;
							sendMessage = oss.str();
							try
							{
								client.sendMessage(sendMessage);
								try
								{
									std::string receiveMessage = client.receiveMessage();
									if (receiveMessage != "Product quantity updated Successfuly")
										std::cout << "Error!\n";
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
							catch (std::runtime_error& e) { std::cout << e.what(); }

						}
						std::cout << "\nTotal money paid by customer is: " << totalCost << std::endl;
						cart.clear();
						break;
					}
					case 5:
					{
						std::string salesRepUserName, salesRepPassword;
						std::cout << "\nAdd new sales rep \nEnter user name: ";
						std::cin >> salesRepUserName;
						std::cout << "Sales rep password: ";
						std::cin >> salesRepPassword;
						std::ostringstream oss;
						oss << "Add_new_client " << salesRepUserName << ' ' << salesRepPassword << ' ' << 0 << ' ' << 0;
						std::string sendMessage;
						sendMessage = oss.str();
						try
						{
							client.sendMessage(sendMessage);
							try
							{
								std::string receiveMessage = client.receiveMessage();
								std::cout << std::endl << receiveMessage;
							}
							catch (std::runtime_error& e) { std::cout << e.what(); }
						}
						catch (std::runtime_error& e) { std::cout << e.what(); }
						break;
					}
					case 6:
					{
						std::string userName;
						std::cout << "Delete sales rep. \nEnter user name: ";
						std::cin >> userName;
						std::ostringstream oss;
						oss << "Delete_client " << userName << ' ' << "null" << ' ' << 0 << ' ' << 0;
						std::string sendMessage;
						sendMessage = oss.str();
						try
						{
							client.sendMessage(sendMessage);
							try
							{
								std::string receiveMessage = client.receiveMessage();
								std::cout << std::endl << receiveMessage;
							}
							catch (std::runtime_error& e) { std::cout << e.what(); }
						}
						catch (std::runtime_error& e) { std::cout << e.what(); }
						break;
					}
					case 7:
					{
						std::cout << "\tDelete product \nEnter product name: ";
						std::cin >> productName;
						std::ostringstream oss;
						oss << "Delete_product " << productName << ' ' << "null " << 0 << ' ' << 0;
						std::string sendMessage;
						sendMessage = oss.str();
						try
						{
							client.sendMessage(sendMessage);
							try
							{
								std::string receiveMessage = client.receiveMessage();
								std::cout << std::endl << receiveMessage;
							}
							catch (std::runtime_error& e) { std::cout << e.what(); }
						}
						catch (std::runtime_error& e) { std::cout << e.what(); }
						break;
					}
					case 8:
					{
						std::cout << "\nLoging admin out!!!\n";
						break;
					}
					default:
					{
						std::cout << "Wrong Input!!! Try again.\n";
						break;
					}
					}
				} while (adminAction != 8);
				break;
			}
			case 2:
			{
				std::string loginDetail;
				int salesRepPassword;
				bool salesRepLogedIn = false;
				int loginTrial = 0;

				do
				{
					loginDetail = promptSalesRepLogin();
					std::ostringstream oss;
					oss << "Client_login " << loginDetail << ' ' << "0 " << "0";
					std::string sendMessage;
					sendMessage = oss.str();
					try
					{
						client.sendMessage(sendMessage);
						try
						{
							std::string receiveMessage = client.receiveMessage();
							if (receiveMessage == "Login_Sucessful")
							{
								std::cout << receiveMessage;
								int action;
								do
								{
									action = promptSalesRep();
									switch (action)
									{
									case 1:
									{
										int productQuantity_s;
										std::cout << "Enter product details: \nEnter product name: ";
										std::cin >> productName;
										std::cout << "Enter product quantity: ";
										std::cin >> productQuantity;
										std::ostringstream oss;
										oss << "Retrieve_product " << productName << ' ' << "null " << productQuantity << ' ' << productPrice;
										std::string sendMessage;
										sendMessage = oss.str();
										try
										{
											client.sendMessage(sendMessage);
											try
											{
												std::string receiveMessage = client.receiveMessage();
												std::istringstream iss(receiveMessage);
												std::string command;
												iss >> command >> productName >> productQuantity_s >> productPrice;
												if (command == "Product_found")
												{
													if (productQuantity_s >= productQuantity)
													{
														std::cout << "Product available and instock.";
														std::ostringstream oss;
														oss << productName << ' ' << productQuantity << ' ' << productPrice << ' ' << productQuantity_s;
														cart.push_back(oss.str());
														std::cout << "\nProduct added to cart.";
													}
													else
														std::cout << "Product out of stock. Available quantity is: " << productQuantity_s << std::endl;
												}
												else
													std::cout << "Product not found!!!";
											}
											catch (std::runtime_error& e) { std::cout << e.what(); }

										}
										catch (std::runtime_error& e) { std::cout << e.what(); }
										break;
									}
									case 2:
									{
										std::cout << "Products to pay for:\n"
											<< "Name\t\tQuantity\t\tPrice\t\tAmount\n";
										double totalCost = 0, amount = 0;
										int productQuantity_s, productQuantity_a;
										for (auto& goods : cart)
										{
											std::istringstream iss(goods);
											iss >> productName >> productQuantity >> productPrice >> productQuantity_s;
											amount = productQuantity * productPrice;
											totalCost += amount;
											std::cout << productName << "\t\t" << productQuantity << "\t\t"
												<< productPrice << "\t\t" << amount << "\n";
											productQuantity_a = productQuantity_s - productQuantity;
											std::ostringstream oss;
											oss << "Update_quantity " << productName << " null " << productQuantity_a << ' ' << productPrice;
											std::string sendMessage;
											sendMessage = oss.str();
											try
											{
												client.sendMessage(sendMessage);
												try
												{
													std::string receiveMessage = client.receiveMessage();
													if (receiveMessage != "Product quantity updated Successfuly")
														std::cout << "Error!\n";
												}
												catch (std::runtime_error& e) { std::cout << e.what(); }
											}
											catch (std::runtime_error& e) { std::cout << e.what(); }

										}
										std::cout << "\nTotal money paid by customer is: " << totalCost << std::endl;
										cart.clear();
										break;
									}
									case 3:
									{
										std::cout << "Logging out";
										loginTrial = 3;
										break;
									}
									default:
									{
										std::cout << "Wrong input... Try again!";
										break;
									}
									}
								} while (action != 3);
							}
							else
							{
								std::cout << receiveMessage;
								loginTrial++;
							}
						}
						catch (std::runtime_error& e) { std::cout << e.what(); }
					}
					catch (std::runtime_error& e) { std::cout << e.what(); }
				} while (loginTrial < 3);
				
				break;
			}
			case 3:
			{
				std::cout << "Exiting!!!\n";
				break;
			}
			default:
			{
				std::cout << "Incrrect Input... Try again.\n";
				break;
			}
			}
		} while (actionLogin != 3);
	}
	catch (std::runtime_error& e) { std::cout << e.what(); }

	return 0;
}