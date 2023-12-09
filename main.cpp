#include <iostream>
#include <iomanip>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
using namespace sql;
using namespace std;

class Customer
{
private:
	int id;
	string name;
	string phone;
	string address;
	int roomNumber;
	double bill;
	double payment_advance;
	int status;
	int days = 0;
	Connection *con;
	Customer(Connection *dbConnection) : con(dbConnection), id(0), roomNumber(0), status(0) {}
	void accept();			   // ACCEPT CUSTOMER DETAILS
	void display();			   // DISPLAY CUSTOMER DETAILS
	void insertIntoDatabase(); // Inserting values into database
	friend class room;
	friend class Hotel;
};

void Customer::accept()
{
	cout << "Enter customer ID: ";
	cin >> id;
	cout << "Enter customer name: ";
	cin.ignore();
	getline(cin, name);
	cout << "Enter customer phone: ";
	cin >> phone;
	cout << "Enter customer address: ";
	cin.ignore();
	getline(cin, address);
}

void Customer::display()
{
	cout << id << "\t\t";
	cout << "| " << left << setfill(' ') << setw(30) << name;
	cout << "| " << phone << "\t\t\t";
	cout << "| " << left << setfill(' ') << setw(30) << address;
	cout << "| " << roomNumber << "\t\t\t";
	if (status == 1)
	{
		cout << "|\t\t-\t\t|" << endl;
	}
	else
	{
		cout << "|\tCHECKED OUT.\t\t|" << endl;
	}
}

void Customer::insertIntoDatabase()
{
	PreparedStatement *pstmt;
	pstmt = con->prepareStatement("INSERT INTO customers (id, name, address, phone, roomNumber, bill, payment_advance, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
	pstmt->setInt(1, id);
	pstmt->setString(2, name);
	pstmt->setString(3, address);
	pstmt->setString(4, phone);
	pstmt->setInt(5, roomNumber);
	pstmt->setDouble(6, bill);
	pstmt->setDouble(7, payment_advance);
	pstmt->setInt(8, status);
	pstmt->executeUpdate();
}

class room
{
private:
	char type;
	char stype;
	char ac;
	int roomNumber;
	double rent;
	int status = 0;
	Connection *con;

public:
	room(Connection *dbConnection) : con(dbConnection) {}
	friend class Hotel;
	void acceptroom(int rno); // ADD ROOMS TO HOTEL DATABASE
	void displayroom();		  // DISPLAY ROOMS
	room()					  // DEFAULT CONSTRUCTOR
	{
		type = '\0';
		stype = '\0';
		ac = '\0';
		roomNumber = 0;
		rent = 0;
		status = 0;
	}

	void insertIntoDatabase()
	{
		try
		{
			PreparedStatement *pstmt;
			pstmt = con->prepareStatement("INSERT INTO rooms VALUES (?, ?, ?, ?, ?, ?)");
			pstmt->setInt(1, roomNumber);
			pstmt->setString(2, string(1, type));
			pstmt->setString(3, string(1, stype));
			pstmt->setString(4, string(1, ac));
			pstmt->setDouble(5, rent);
			pstmt->setInt(6, status);
			pstmt->executeUpdate();
			delete pstmt;
		}
		catch (SQLException &e)
		{
			// Handle SQL exception
			cerr << "SQL Error: " << e.what() << endl;
			exit(1);
		}
	}
};

void room::acceptroom(int rno)
{
	roomNumber = rno;

	cout << "Type AC/Non-AC (A/N) : ";
	cin >> ac;
	while (ac != 'A' && ac != 'N')
	{
		cout << "Please Re-Enter Type: AC/Non-AC (A/N) : ";
		cin >> ac;
	}

	cout << "Type Comfort Suite or Normal (S/N) : ";
	cin >> type;
	while (type != 'S' && type != 'N')
	{
		cout << "Please Re-enter Type Comfort Suite or Normal (S/N) : ";
		cin >> type;
	}

	cout << "Type Size (B/S) : ";
	cin >> stype;
	while (stype != 'B' && stype != 'S')
	{
		cout << "Please Re-enter Type Size (B/S) : ";
		cin >> stype;
	}

	cout << "Daily Rent : ";
	cin >> rent;
	while (rent < 0 || rent > 20000)
	{
		cout << "Please enter valid rent.";
		cin >> rent;
	}

	status = 0; // Booking status of the room

	insertIntoDatabase();
	cout << "\nRoom Added Successfully!" << endl;
}

void room::displayroom()
{
	PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT * FROM rooms WHERE roomNumber = ?");
	pstmt->setInt(1, roomNumber);
	ResultSet *res = pstmt->executeQuery();

	if (res->next())
	{
		cout << "| " << res->getInt("roomNumber") << ".\t\t|\t" << res->getString("ac") << "\t\t|\t" << res->getString("type") << "\t\t|\t"
			 << res->getString("stype") << "\t\t|\t" << res->getDouble("rent") << "\t\t|\t";
		if (res->getInt("status") == 0)
		{
			cout << "Available.\t|";
		}
		else
		{
			cout << "Booked.\t\t|";
		}
		cout << endl;
	}
	else
	{
		cout << "Room details not found." << endl;
	}

	delete pstmt;
	delete res;
}

class Hotel
{
private:
	myMySQL_Driver *driver;
	Connection *con;

public:
public:
	void addRooms();	 // ADD ROOMS TO DATABASE
	void searchroom();	 // SEARCH FOR A PARTICULAR ROOM
	void CheckIn();		 // FOR CUSTOMER CHECKIN
	void searchcust();	 // SEARCH WHETHER A PARTICULAR CUSTOMER IS STAYING AT THE HOTEL
	void availability(); // CHECK AVAILABILITY OF ROOMS
	void CheckOut();	 // CHECKOUT AND BILLING PROCEDURE
	void Summary();		 // GUEST SUMMARY
	bool isAdmin;
	Hotel()
	{
		// Initializing MySQL Connector/C++
		driver = myget_mysql_driver_instance();
		con = driver->connect("HostName-PORT", "Username", "Password"); // Host Name , Username , Password
		// Connect to the specific database
		con->setSchema("Database_Name"); // Database Name "Database_Name"
		// Create the 'rooms' table if it doesn't exist
		Statement *stmt = con->createStatement();
		stmt->execute("CREATE TABLE IF NOT EXISTS rooms ("
					  "roomNumber INT PRIMARY KEY, "
					  "type CHAR(1), "
					  "stype CHAR(1), "
					  "ac CHAR(1), "
					  "rent DOUBLE, "
					  "status INT)");
		delete stmt;
	}

	void authenticateAdmin() // Authentication for admin
	{
		string username, password;
		cout << "Enter admin username: ";
		cin >> username;
		cout << "Enter admin password: ";
		cin >> password;

		if (username == "admin" && password == "pass")
		{
			isAdmin = true;
			cout << "Authentication successful. Welcome, admin!" << endl;
		}
		else
		{
			isAdmin = false;
			cout << "Authentication failed. You are not authorized." << endl;
		}
	}
	~Hotel()
	{
		delete con;
	}
};

void Hotel::addRooms() // Add rooms to database....
{
	room room(con);
	int rno;
	cout << "Enter room number: ";
	cin >> rno;
	while (rno <= 0)
	{
		cout << "Invalid. Enter valid room number: ";
		cin >> rno;
	}
	room.acceptroom(rno);
}

void Hotel::searchroom() // Searching rooms in database....
{
	room room(con);

	int flag = 0;
	char ac, type, stype;

	cout << "Do you want AC or Non-AC? (A/N): ";
	cin >> ac;
	cout << "Do you want suite or normal room? (S/N): ";
	cin >> type;
	cout << "Size? (B/S): ";
	cin >> stype;

	cout << "Search Result:" << endl;
	cout << "| Room No.\t|\tAC/Non-AC\t|\tType\t\t|\tStype\t\t|\tRent\t\t|\tAvailability  \t|" << endl;

	PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT * FROM rooms WHERE ac = ? AND type = ? AND stype = ?");
	pstmt->setString(1, string(1, ac));
	pstmt->setString(2, string(1, type));
	pstmt->setString(3, string(1, stype));
	ResultSet *res = pstmt->executeQuery();

	while (res->next())
	{
		cout << "| " << res->getInt("roomNumber") << ".\t\t|\t" << res->getString("ac") << "\t\t|\t"
			 << res->getString("type") << "\t\t|\t" << res->getString("stype") << "\t\t|\t" << res->getDouble("rent") << "\t\t|\t";
		if (res->getInt("status") == 0)
		{
			cout << "Available.\t|";
		}
		else
		{
			cout << "Booked.\t\t|";
		}
		cout << endl;
		flag = 1;
	}

	if (flag == 0)
	{
		cout << "No matching rooms found." << endl;
	}

	delete pstmt;
	delete res;
}

void Hotel::CheckIn()
{
	room room(con);
	Customer customer(con);
	int rno;

	cout << "Enter room number: ";
	cin >> rno;

	// Check if the entered room number is valid
	if (rno <= 0)
	{
		cout << "Invalid room number. Please enter again." << endl;
		return;
	}

	// Check if the room is unoccupied
	PreparedStatement *pstmtCheckRoom;
	pstmtCheckRoom = con->prepareStatement("SELECT * FROM rooms WHERE roomNumber = ? AND status = 0");
	pstmtCheckRoom->setInt(1, rno);
	ResultSet *resCheckRoom = pstmtCheckRoom->executeQuery();

	if (resCheckRoom->next())
	{
		char ch2;
		cout << "Room available." << endl;
		room.roomNumber = rno;
		room.displayroom();
		cout << "Do you wish to continue? Press (Y/n)";
		cin >> ch2;

		if (ch2 == 'Y' || ch2 == 'y')
		{
			customer.accept();
			cout << "Enter number of days of stay: ";
			cin >> customer.days;

			// Accessing rent from the database
			PreparedStatement *pstmtRent;
			pstmtRent = con->prepareStatement("SELECT rent FROM rooms WHERE roomNumber = ?");
			pstmtRent->setInt(1, rno);
			ResultSet *resRent = pstmtRent->executeQuery();

			if (resRent->next())
			{
				customer.bill = customer.days * resRent->getDouble("rent");
				cout << "Your total bill will be Rs." << customer.bill << ". Min adv payment=" << customer.bill / 4 << " What will you be paying?";
				cin >> customer.payment_advance;

				while (customer.payment_advance < customer.bill / 4 || customer.payment_advance > customer.bill)
				{
					cout << "Enter a valid amount.";
					cin >> customer.payment_advance;
				}

				cout << "Thank you. Booking confirmed." << endl;
				cout << "--------------------------------------------------------------" << endl;

				PreparedStatement *pstmtUpdateRoom; // Updating room status to booked
				pstmtUpdateRoom = con->prepareStatement("UPDATE rooms SET status = 1 WHERE roomNumber = ?");
				pstmtUpdateRoom->setInt(1, rno);
				pstmtUpdateRoom->executeUpdate();
				customer.roomNumber = rno;
				customer.status = 1;
				customer.insertIntoDatabase(); // Inserting customer details into the database
			}
			else
			{
				cout << "Error fetching rent from the database." << endl;
			}

			delete pstmtRent;
			delete resRent;
		}
	}
	else
	{
		cout << "Room is occupied. Please choose another room." << endl;
	}

	delete pstmtCheckRoom;
	delete resCheckRoom;
}

void Hotel::CheckOut()
{
	int rno;
	cout << "Enter room number: ";
	cin >> rno;

	// Check if the entered room number is valid
	if (!rno)
	{
		cout << "Invalid room number. Please enter again." << endl;
		return;
	}

	// Fetching customer details from the database
	PreparedStatement *pstmtCust;
	pstmtCust = con->prepareStatement("SELECT * FROM customers WHERE roomNumber = ? AND status = 1");
	pstmtCust->setInt(1, rno);
	ResultSet *resCust = pstmtCust->executeQuery();

	if (resCust->next())
	{
		cout << "CHECKING OUT." << endl;

		// Displaying customer details
		cout << "| " << resCust->getString("name") << "\t\t|\t" << resCust->getString("phone") << "\t\t|\t" << resCust->getString("address") << "\t\t|";

		double totalBill = resCust->getDouble("bill");
		double advPayment = resCust->getDouble("payment_advance");

		cout << "Your total bill is Rs." << totalBill << endl;
		cout << "Adv payment: Rs." << advPayment << endl;
		cout << "Hence, pending payment = Rs." << totalBill - advPayment << endl;

		// Updating room status to unoccupied
		PreparedStatement *pstmtRoom;
		pstmtRoom = con->prepareStatement("UPDATE rooms SET status = 0 WHERE roomNumber = ?");
		pstmtRoom->setInt(1, rno);
		pstmtRoom->executeUpdate();

		// Updating customer status to checked out
		PreparedStatement *pstmtUpdateCust;
		pstmtUpdateCust = con->prepareStatement("UPDATE customers SET status = 0 WHERE roomNumber = ?");
		pstmtUpdateCust->setInt(1, rno);
		pstmtUpdateCust->executeUpdate();

		cout << "Thank you! Visit Again :)" << endl;
	}
	else
	{
		cout << "No customer found for the specified room." << endl;
	}

	delete pstmtCust;
	delete resCust;
}

void Hotel::searchcust()
{
	int id;
	cout << "Enter customer ID: ";
	cin >> id;

	PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT * FROM customers WHERE id = ?");
	pstmt->setInt(1, id);
	ResultSet *res = pstmt->executeQuery();

	cout << "    Name";
	cout << "\t\t Phone";
	cout << "\t\t\t Address" << endl;

	if (res->next())
	{
		cout << "| " << res->getString("name") << "\t\t|\t" << res->getString("phone") << "\t\t|\t" << res->getString("address") << "\t\t|";
	}
	else
	{
		cout << "No customer found for the specified ID." << endl;
	}

	delete pstmt;
	delete res;
}

void Hotel::availability() // Checking the avalable room from database
{
	cout << "The list of all available rooms:" << endl;
	cout << "| Room No.\t|\tAC/Non-AC\t|\tType\t\t|\tStype\t\t|\tRent\t\t|\tAvailability  \t|" << endl;

	PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT * FROM rooms");
	ResultSet *res = pstmt->executeQuery();
	while (res->next())
	{
		cout << "| " << res->getInt("roomNumber") << ".\t\t|\t" << res->getString("ac") << "\t\t|\t"
			 << res->getString("type") << "\t\t|\t" << res->getString("stype") << "\t\t|\t" << res->getDouble("rent") << "\t\t|\t";
		if (res->getInt("status") == 0)
		{
			cout << "Available.\t|";
		}
		else
		{
			cout << "Booked.\t\t|";
		}
		cout << endl;
	}

	delete pstmt;
	delete res;
}

void Hotel::Summary()
{
	cout << "Guest Summary:" << endl;
	cout << "===================================================================================================================================================================================================" << endl;
	cout << "|   ID     |   Name                             |   Address                          |   Phone        |   Room No.     |   Bill                 |     Advance Payment    |     Status      |" << endl;
	cout << "===================================================================================================================================================================================================" << endl;

	PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT * FROM customers");

	ResultSet *res = pstmt->executeQuery();

	while (res->next())
	{
		cout << "|   " << setw(4) << res->getInt("id")
			 << "   |   " << left << setw(30) << res->getString("name")
			 << "   |   " << left << setw(30) << res->getString("address")
			 << "   |   " << setw(10) << res->getInt("phone")
			 << "   |   " << setw(10) << res->getDouble("roomNumber")
			 << "   |   " << setw(18) << res->getDouble("bill")
			 << "   |   " << setw(18) << res->getDouble("payment_advance")
			 << "   |   " << setw(8) << (res->getInt("status") == 1 ? "Checked In" : "Checked Out") << "   |" << endl;
	}

	if (res->rowsCount() == 0)
	{
		cout << "No customers found." << endl;
	}

	delete pstmt;
	delete res;
}

int main()
{
	Hotel downtown;
	bool isAdmin = false;
	char ch;
	int adminchoice;

	do
	{
		cout << endl
			 << "======================================================================================WELCOME TO DOWNTOWN HOTEL======================================================================================";
		cout << endl
			 << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
		cout << endl
			 << "\t\t\t\t\t\t\t\t\t\t\t"
				"MENU:\n\t\t\t\t\t\t\t\t\t\t\t"
				"1. OPERATE AS ADMIN\n\t\t\t\t\t\t\t\t\t\t\t"
				"2. OPERATE AS CUSTOMER\n\t\t\t\t\t\t\t\t\t\t\t"
				"3. EXIT\n\t\t\t\t\t\t\t\t\t\t\t"
				"Enter your choice:";
		cin >> ch;

		switch (ch)
		{
		case '1':
			if (!isAdmin)
			{
				downtown.authenticateAdmin();
			}
			if (downtown.isAdmin)
			{
				cout << "Enter your choice - " << endl;
				cout << "1) Add rooms " << endl;
				cout << "2) View guest summary " << endl;
				cin >> adminchoice;
				switch (adminchoice)
				{
				case 1:
					cout << "Add database of rooms in the hotel:" << endl;
					downtown.addRooms();
					cout << "Database updated. Going back to the main menu." << endl;
					break;
				case 2:
					downtown.Summary();
					break;
				default:
					cout << "Invalid Choice." << endl;
					break;
				}
			}

			break;
		case '2':
			char ch1;
			do
			{
				cout << endl
					 << "======================================================================================WELCOME TO DOWNTOWN HOTEL======================================================================================";
				cout << endl
					 << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
				cout << "\n\t\t\t\t\t\t\t\t\t\t\t"
						"Menu:\n\t\t\t\t\t\t\t\t\t\t\t"
						"1. Check Availability of rooms.\n\t\t\t\t\t\t\t\t\t\t\t"
						"2. Search Room\n\t\t\t\t\t\t\t\t\t\t\t"
						"3. Check In\n\t\t\t\t\t\t\t\t\t\t\t"
						"4. Search Customer\n\t\t\t\t\t\t\t\t\t\t\t"
						"5. CheckOut\n\t\t\t\t\t\t\t\t\t\t\t"
						"6. Go back to Main Menu.\n\t\t\t\t\t\t\t\t\t\t\t"
						"Enter your choice:";
				cin >> ch1;

				switch (ch1)
				{
				case '1':
					downtown.availability();
					break;

				case '2':
					downtown.searchroom();
					break;

				case '3':
					downtown.CheckIn();
					break;

				case '4':
					downtown.searchcust();
					break;

				case '5':
					downtown.CheckOut();
					break;

				case '6':
					break;

				default:
					cout << "Invalid Choice." << endl;
					break;
				}
			} while (ch1 != '6');
			break;

		case '3':
			cout << "Thank you!";
			break;

		default:
			cout << "Invalid Choice." << endl;
			break;
		}
	} while (ch != '3');
	return 0;
}
