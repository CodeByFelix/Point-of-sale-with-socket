# Point-of-sale-with-socket
I have done a previous version of this point of sale software earlier. 
In this version, I created a server which runs the database and stores all the necessary files, and a client app which connects with the server through socket connection. Multiple client app can be connected to the server at same time. 
The server stores all the data the software will work with.
All logins, and sales are done from the client side.
The client software will have to connect to the server before any operation could be performed.
The login has two privileged: Admin and sales rep.

the Admin login detial is fixed with the software while the admin can add as many sales rep as possible. 
The admin can add sales rep, delete sales rep, view sales rep on record, add product, delete product, view available products, and sale products.
The sales rep can only sale product.

The basic resources used in this software inclludes: winsock2, file stream, vector container, string stream and algorithm.

This software is a blueprint to a real application POS being used in malls.
