#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#pragma comment (lib, "ws2_32.lib")

class SocketServer
{
private:
	int portNumber;
	WSADATA wsaData;
	SOCKET server;
	sockaddr_in serverAddr, clientAddr;
	static const int bufferSize = 1024;
	char buffer[bufferSize];
	fd_set masterConnection;

public:
	explicit SocketServer(const int& _portNumber) : portNumber(_portNumber) {}

	~SocketServer()
	{
		std::cout << "Socket Closing and Cleanup.\n";
		FD_CLR(server, &masterConnection);
		closesocket(server);
		while (masterConnection.fd_count > 0)
		{
			SOCKET closeSocket = masterConnection.fd_array[0];
			FD_CLR(closeSocket, &masterConnection);
			closesocket(closeSocket);
		}
		WSACleanup();
	}

	//Creating Socket
	void creatSocket()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			throw std::runtime_error("Winsock did not startup\n");
		}

		server = socket(AF_INET, SOCK_STREAM, 0);
		if (server == INVALID_SOCKET)
		{
			WSACleanup();
			throw std::runtime_error("Socket Creation Failed\n");
		}

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(portNumber);
		if (bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			closesocket(server);
			WSACleanup();
			throw std::runtime_error("Socket Binding Failed\n");
		}

		std::cout << "Socket Created Successfult\n";
	}

	void listenOut() const noexcept
	{
		listen(server, SOMAXCONN);
		FD_ZERO(&masterConnection);
		FD_SET(server, &masterConnection);
	}

	[[nodiscard]] std::string acceptData()
	{
		fd_set copyConnection = masterConnection;
		//check who is talking
		int incoming = select(FD_SETSIZE, &copyConnection, nullptr, nullptr, nullptr);
		//iterate incomming 
		for (int i = 0; i < incoming; i++)
		{
			SOCKET incomingSocket = copyConnection.fd_array[i];
			if (incomingSocket == server)
			{
				SOCKET client = accept(server, nullptr, nullptr);
				if (client == INVALID_SOCKET)
					throw std::runtime_error("Fail to accept connection\n");
				else
				{
					FD_SET(client, &masterConnection);
					std::ostringstream oss;
					oss << "New_connection " << "null " << "null " << "0 " << "0.0" << ' ' << client;
					//std::string message = oss.str();
					//return message;
					return oss.str();

				}
			}
			else
			{
				//Receive Message
				int bytesReceived = recv(incomingSocket, buffer, bufferSize, 0);
				if (bytesReceived <= 0)
				{
					FD_CLR(incomingSocket, &masterConnection);
					closesocket(incomingSocket);
					throw std::runtime_error("Client Disconnected\n");
				}
				else
				{
					//Manage the messages here.
					//std::cout << "Received message from " << incomingSocket << std::endl;
					std::ostringstream oss;
					oss << buffer << ' ' << incomingSocket;
					return oss.str();
				}
					

			}
		}

	}

	void sendMessage(std::string message, SOCKET client)
	{
		if (send(client, message.c_str(), message.length() + 1, 0) == SOCKET_ERROR)
			throw std::runtime_error("Fail to send message.\n");
		int bytesReceived = recv(client, buffer, bufferSize, 0);
		if (bytesReceived <= 0)
		{
			FD_CLR(client, &masterConnection);
			closesocket(client);
			throw std::runtime_error("Client Disconnected\n");
		}
		else
		{
			std::string buff;
			buff = buffer;
			if (buff == "OK")
				std::cout << "Message sent.\n";
			else
				std::cout << "Message not sent.\n";
		}
		
	}

};

class Product
{
private:
	std::string productName;
	//int productQuantity;
	double productPrice;

public:
	int productQuantity;
	explicit Product() = default;
	explicit Product(std::string _productName, int _productQuantity, double _productPrice) :
		productName(_productName),
		productQuantity(_productQuantity),
		productPrice(_productPrice) {}

	[[nodiscard]] std::string getProductName() const noexcept { return productName; }
	[[nodiscard]] int getProductQuantity() const noexcept { return productQuantity; }
	[[nodiscard]] double getProductPrice() const noexcept { return productPrice; }

	void updateQuantity(int quantity)
	{
		productQuantity = quantity;
	}

	//Serialization
	friend std::ostream& operator<< (std::ostream& os, const Product& product)
	{
		return os << product.productName << ' '
			<< product.productPrice << ' '
			<< product.productQuantity;
	}

	friend std::istream& operator >> (std::istream& is, Product& product)
	{
		return is >> product.productName
			>> product.productPrice
			>> product.productQuantity;
	}
};

class DataBase
{
private:
	std::vector <Product> products;
	std::vector <std::string> clients;
	std::string storeName;

public:
	explicit DataBase() = default;
	explicit DataBase(std::string& _storeName) : storeName(_storeName) {}

	[[nodiscard]] const auto& getAllProducts() const noexcept { return products; }
	void addProduct(const Product& product) { products.push_back(product); }

	[[nodiscard]] auto& getProduct(std::string productName)
	{
		auto it = std::find_if(products.begin(), products.end(), [productName](const Product& product)
			{return product.getProductName() == productName;
			});

		if (it != products.end()) { return *it; }
		else
		{
			throw std::runtime_error("Product Not Found!\n");
		}

	}

	void deleteProduct(std::string productName)
	{
		auto it = std::find_if(products.begin(), products.end(), [productName](const Product& product)
			{return product.getProductName() == productName;
			});

		if (it != products.end()) { products.erase(it); }
		else
		{
			throw std::runtime_error("Product Not Found!\n");
		}

	}
	

	void saveToProductFile() const
	{
		std::ofstream file(storeName, std::ios::binary);
		if (!file)
			throw std::runtime_error("File fail to open\n");
		for (const auto& product : products)
		{
			file << product << '\n';
		}
		file.close();
	}

	void loadFromProductFile()
	{
		std::ifstream file(storeName, std::ios::binary);
		if (!file)
			throw std::runtime_error("File fail to open\n");

		products.clear();
		std::string line;
		while (getline(file, line))
		{
			Product product;
			std::istringstream iss(line);
			if (!(iss >> product)) {
				throw std::runtime_error("Fail to read client");
			}

			products.push_back(product);
		}

	}
	void addClient(std::string client) { clients.push_back(client); }

	void deleteClient(std::string client)
	{
		clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
	}

	[[nodiscard]] const auto& getClient(std::string& _userName)
	{
		for (const auto& client : clients)
		{
			std::string userName, password;
			std::istringstream iss(client);
			iss >> userName >> password;
			if (_userName == userName)
				return client;
		}
		throw std::runtime_error("Client not found.\n");
	}

	void saveToClientFile()
	{
		std::ofstream file("Client_file.txt", std::ios::binary);
		if (!file)
			throw std::runtime_error("File fail to open\n");
		for (const auto& client : clients)
		{
			file << client << '\n';
		}
		file.close();
	}

	void loadFromClientFile()
	{
		std::ifstream file("Client_file.txt", std::ios::binary);
		if (!file)
			throw std::runtime_error("File fail to open\n");

		clients.clear();
		std::string line;
		while (getline(file, line))
		{
			clients.push_back(line);
		}
	}

};

inline int prompt()
{
	int action;
	std::cout << "\n\t\tPoint of sale Server.\n"
		<< "\tEnter Action\n"
		<< "\t1 -> Start Server.\n"
		<< "\t2 -> Exit.\n"
		<< "Input: ";
	std::cin >> action;
	return action;
}

int main()
{
	std::string storeName = "Products.txt";
	DataBase dataBase(storeName);
	try 
	{
		dataBase.loadFromProductFile();
	}
	catch (std::runtime_error& e) { std::cout << e.what(); }
	try
	{
		dataBase.loadFromClientFile();
	}
	catch (std::runtime_error& e) { std::cout << e.what(); }
	
	int portNumber;
	int action;
	std::string command, string1, string2, adminUserName = "Admin", adminPassword = "12345678";
	double price;
	int quantity;
	SOCKET client;
	do
	{
		action = prompt();
		switch (action)
		{
		case 1:
		{
			std::cout << "Enter Port Number to run Server: ";
			std::cin >> portNumber;
			SocketServer server(portNumber);
			try
			{
				server.creatSocket();
				server.listenOut();
				bool running = true;
				while (running)
				{
					try
					{
						std::string message = server.acceptData();
						std::istringstream iss(message);
						iss >> command >> string1 >> string2 >> quantity >> price >> client;
						//std::cout << "Client is " << command << ' ' << string1 << ' ' << string2 << ' ' << price << ' ' << quantity << ' ' << client << std::endl;
						if (command == "New_connection")
						{
							std::cout << "New client " << client << " Added.\n";
						}
						else if (command == "Client_login")
						{
							try
							{
								const auto& loginDetail = dataBase.getClient(string1);
								std::istringstream iss(loginDetail);
								std::string userName, password;
								iss >> userName >> password;
								if (string1 == userName && string2 == password)
								{
									std::string sendMessage;
									sendMessage = "Login_Sucessful";
									try
									{
										server.sendMessage(sendMessage, client);
									}
									catch (std::runtime_error& e) { std::cout << e.what(); }
								}
								else
								{
									std::string sendMessage;
									sendMessage = "Incorrect_Login_Details";
									try
									{
										server.sendMessage(sendMessage, client);
									}
									catch (std::runtime_error& e) { std::cout << e.what(); }
								}
							}
							catch (std::runtime_error& e)
							{
								std::cout << e.what();
								std::string sendMessage;
								sendMessage = "User_Not_Found";
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
							
						}
						else if (command == "Admin_login")
						{
							if (string1 == "Admin" && string2 == "12345678")
							{
								std::string sendMessage;
								sendMessage = "Login_Successful";
								//std::cout << sendMessage << std::endl;
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
								
							}
							else
							{
								std::string sendMessage;
								sendMessage = "Incorrect_Login_Details";
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
						}
						else if (command == "Add_new_client")
						{
							try
							{
								const auto& loginDetails = dataBase.getClient(string1);
								std::string sendMessage = "Client already exist";
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
							catch (std::runtime_error& e)
							{
								std::ostringstream oss;
								oss << string1 << ' ' << string2;
								dataBase.addClient(oss.str());
								dataBase.saveToClientFile();
								std::string sendMessage = "New Client Added Successfuly.";
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
							
						}
						else if (command == "Delete_client")
						{
							try
							{
								const auto& loginDetails = dataBase.getClient(string1);
								dataBase.deleteClient(loginDetails);
								try
								{
									dataBase.saveToClientFile();
									std::string sendMessage = "Sales rep deleted successfuly";
									try
									{
										server.sendMessage(sendMessage, client);
									}
									catch (std::runtime_error& e) { std::cout << e.what(); }
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
							catch (std::runtime_error& e)
							{
								std::string sendMessage = "Sales rep not fount";
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
						}
						else if (command == "Add_product")
						{
							try
							{
								auto& product = dataBase.getProduct(string1);
								std::string sendMessage = "Product alredy in stock";
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
							catch (std::runtime_error& e)
							{
								dataBase.addProduct(Product(string1, quantity, price));
								try
								{
									dataBase.saveToProductFile();
									std::string sendMessage = "Product added successfuly";
									try
									{
										server.sendMessage(sendMessage, client);
									}
									catch (std::runtime_error& e) { std::cout << e.what(); }
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
						}
						else if (command == "Retrieve_product")
						{
							try
							{
								auto& product = dataBase.getProduct(string1);
								string1 = product.getProductName();
								quantity = product.getProductQuantity();
								price = product.getProductPrice();
								command = "Product_found";
								std::ostringstream oss;
								oss << command << ' ' << string1 << ' ' << quantity << ' ' << price;
								std::string sendMessage;
								sendMessage = oss.str();
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
							catch (std::runtime_error& e) 
							{
								command = "Product_not_found";
								std::ostringstream oss;
								oss << command << ' ' << string1 << ' ' << quantity << ' ' << price;
								std::string sendMessage;
								sendMessage = oss.str();
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
						}
						else if (command == "Update_quantity") //Update_quantity
						{
							try
							{
								auto& product = dataBase.getProduct(string1);
								product.updateQuantity(quantity);
								try
								{
									dataBase.saveToProductFile();
									std::string sendMessage = "Product quantity updated Successfuly";
									try
									{
										server.sendMessage(sendMessage, client);
									}
									catch (std::runtime_error& e) { std::cout << e.what(); }
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
								
							}
							catch (std::runtime_error& e)
							{
								std::string sendMessage = "Product not found";
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
						}
						else if (command == "Get_all_product")
						{
							const auto& products = dataBase.getAllProducts();
							for (const auto& product : products)
							{
								std::ostringstream oss;
								oss << "product " << product.getProductName() << ' '
									<< product.getProductQuantity() << ' '
									<< product.getProductPrice();
								std::string sendMessage;
								sendMessage = oss.str();
								std::cout << sendMessage << std::endl;
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
							std::ostringstream oss;
							oss << "sent " << "null " << 0 << ' ' << 0;
							std::string sendMessage;
							sendMessage = oss.str();
							try
							{
								server.sendMessage(sendMessage, client);
							}
							catch (std::runtime_error& e) { std::cout << e.what(); }
						}
						else if (command == "Delete_product")
						{
							try
							{
								dataBase.deleteProduct(string1);
								try
								{
									dataBase.saveToProductFile();
									std::string sendMessage = "Product deleted";
									try
									{
										server.sendMessage(sendMessage, client);
									}
									catch (std::runtime_error& e) { std::cout << e.what(); }
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
								
							}
							catch (std::runtime_error& e)
							{
								std::string sendMessage = "Product not found";
								try
								{
									server.sendMessage(sendMessage, client);
								}
								catch (std::runtime_error& e) { std::cout << e.what(); }
							}
						}
						else
						{
							std::cout << "Unknown Command\n";
							std::string sendMessage = "Unknown Command";
							try
							{
								server.sendMessage(sendMessage, client);
							}
							catch (std::runtime_error& e) { std::cout << e.what(); }
						}
					}
					catch (std::runtime_error& e) { std::cout << e.what(); }
				}
				
			}
			catch (std::runtime_error& e) { std::cout << e.what(); }
			break;
		}
		case 2:
		{
			std::cout << "\nServer Shuting down...\n";
			break;
		}
		default:
		{
			std::cout << "Wrong input. Try again.";
			break;
		}
		}

	} while (action != 2);
}