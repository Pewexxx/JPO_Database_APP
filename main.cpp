#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <iomanip>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Text_Display.H>

using namespace std;

class Address {
public:
    string city{};
    string street{};
    int house{};
    string flat{};
};

class Book {
public:
    int id{};
    string title{};
    string author{};
    int yearPublished{};
    bool isBorrowed{};
};

class User {
public:
    int id{};
    string forename{};
    string surname{};
    Address address{};
    vector<int> borrowedBooksID;
};

using LibraryUsers = std::vector<User>;
using LibraryBooks = std::vector<Book>;

ostream& recWriteUser(ostream& o, const vector<User>& users) {
    for (const auto& user : users) {
        o << user.id << ' ' << user.forename << ' ' << user.surname << ' '
            << user.address.city << ' ' << user.address.street << ' ' << user.address.house;

        if (!user.address.flat.empty()) {
            o << ' ' << user.address.flat << ' ';
        }
        else {
            o << " - ";
        }

        for (const auto& borrowedBookId : user.borrowedBooksID) {
            o << borrowedBookId << ' ';
        }
        o << "-1\n";
    }
    return o;
}

istream& recReadUser(istream& i, vector<User>& users) {
    User user;
    while (i >> user.id >> user.forename >> user.surname >> user.address.city >> user.address.street
        >> user.address.house) {
        i >> ws;
        if (i.peek() == '-') {
            i.ignore(); 
            user.address.flat = ""; 
        }
        else {
            i >> user.address.flat;
        }
  
        user.borrowedBooksID.clear();

        int borrowedBookId;
        while (i >> borrowedBookId) {
            if (borrowedBookId == -1) {
                i.ignore(); 
                break;
            }

            user.borrowedBooksID.push_back(borrowedBookId);
        }
        users.push_back(user);
    }
    return i;
}

ostream& recWriteBook(ostream& o, const vector<Book>& books) {
    for (const auto& book : books) {
        o << book.id << ';' << book.title << ';' << book.author << ';' << book.yearPublished << ';' << book.isBorrowed << '\n';
    }
    return o;
}

istream& recReadBook(istream& i, vector<Book>& books) {
    Book book;
    while (i >> book.id) {
        i.ignore(); // Ignore the semicolon after id
        getline(i >> std::ws, book.title, ';'); // Read title until the next semicolon
        getline(i >> std::ws, book.author, ';'); // Read author until the next semicolon
        i >> book.yearPublished >> book.isBorrowed;
        books.push_back(book);
    }
    return i;
}

LibraryUsers libraryUsers;
LibraryBooks libraryBooks;
int borrow_BookId = 0;
int borrow_UserId = 0;
const string usersFilePath = "LibraryUsers_Data.txt";
const string booksFilePath = "LibraryBooks_Data.txt";

void BorrowBook(int userId, int bookId, LibraryUsers& libraryUsers, LibraryBooks& libraryBooks);

void ReturnBook(int bookId, LibraryUsers& libraryUsers, LibraryBooks& libraryBooks);

Fl_Window* mainWindow;

Fl_Text_Display* textDisplay;

Fl_Window* addReaderWindow;
Fl_Input* forenameInput;
Fl_Input* surnameInput;
Fl_Input* cityInput;
Fl_Input* streetInput;
Fl_Input* houseInput;
Fl_Input* flatInput;

Fl_Window* addBookWindow;
Fl_Input* titleInput;
Fl_Input* authorInput;
Fl_Input* yearInput;

Fl_Window* rentBookWindow;
Fl_Input* userIdInput;
Fl_Input* bookIdInput;

Fl_Window* returnBookWindow;
Fl_Input* bookIdInput_ret;

void updateTextDisplay(const string& info) {
    textDisplay->buffer()->text(info.c_str());
}

void listReadersCallback(Fl_Widget*, void*) {
    textDisplay->buffer()->text("");

    if (libraryUsers.empty()) {
        updateTextDisplay("No readers available.");
        return;
    }

    string info = "List of Readers:\n";
    info += "ID   Name                  Address                            Borrowed Books\n";
    info += "---- -------------------- ---------------------------------- -------------------------------\n";

    for (const auto& reader : libraryUsers) {
        info += to_string(reader.id) + string(4 - to_string(reader.id).size(), ' ');
        info += reader.forename + " " + reader.surname + string(17 - (reader.forename + " " + reader.surname).size(), ' ');

        info += reader.address.city + ", " + reader.address.street + " ";
        if (!reader.address.flat.empty()) {
            info += to_string(reader.address.house) + "/" + reader.address.flat + string(5 - (to_string(reader.address.house) + "/" + reader.address.flat).size(), ' ');
        }
        else {
            info += to_string(reader.address.house) + string(9 - to_string(reader.address.house).size(), ' ');
        }

        info += "        ";

        if (!reader.borrowedBooksID.empty()) {
            for (const auto& bookID : reader.borrowedBooksID) {
                info += to_string(bookID) + "  ";
            }
        }
        else {
            info += "No borrowed books";
        }
        info += "\n";
    }
    updateTextDisplay(info);
}

void listBooksCallback(Fl_Widget*, void*) {
    textDisplay->buffer()->text("");

    if (libraryBooks.empty()) {
        updateTextDisplay("No books available.");
        return;
    }

    string info = "List of Books:\n";
    info += "ID   Title                    Author                    Pub. Date     Borrowed\n";
    info += "---- --------------------- ----------------------- ---------------- ------------\n";

    for (const auto& book : libraryBooks) {
        info += to_string(book.id) + "   ";
        info += book.title + "  ";
        info += book.author + "        ";
        info += to_string(book.yearPublished) + "        ";
        info += (book.isBorrowed ? "Yes" : "No");

        info += "\n";
    }

    updateTextDisplay(info);
}

void addReaderCallback(Fl_Widget*, void*) {
    addReaderWindow = new Fl_Window(400, 300, "Add Reader");

    forenameInput = new Fl_Input(150, 20, 200, 30, "Forename:");
    surnameInput = new Fl_Input(150, 60, 200, 30, "Surname:");
    cityInput = new Fl_Input(150, 100, 200, 30, "City:");
    streetInput = new Fl_Input(150, 140, 200, 30, "Street:");
    houseInput = new Fl_Input(150, 180, 200, 30, "House:");
    flatInput = new Fl_Input(150, 220, 200, 30, "Flat:");

    Fl_Button* addButton = new Fl_Button(150, 260, 100, 30, "Add Reader");
    addButton->callback([](Fl_Widget*, void*) {
        string forename = forenameInput->value();
        string surname = surnameInput->value();
        string city = cityInput->value();
        string street = streetInput->value();
        int house = atoi(houseInput->value());
        string flat = flatInput->value();

        User newUser;
        newUser.id = libraryUsers.size() + 1;
        newUser.forename = forename;
        newUser.surname = surname;
        newUser.address.city = city;
        newUser.address.street = street;
        newUser.address.house = house;
        newUser.address.flat = flat;

        libraryUsers.push_back(newUser);

        addReaderWindow->hide();
        });

    addReaderWindow->end();
    addReaderWindow->show();
}

void addBookCallback(Fl_Widget*, void*) {
    addBookWindow = new Fl_Window(400, 250, "Add Book");

    titleInput = new Fl_Input(150, 20, 200, 30, "Title:");
    authorInput = new Fl_Input(150, 60, 200, 30, "Author:");
    yearInput = new Fl_Input(150, 100, 200, 30, "Year:");

    Fl_Button* addButton2 = new Fl_Button(150, 140, 100, 30, "Add Book");
    addButton2->callback([](Fl_Widget*, void*) {
        string title = titleInput->value();
        string author = authorInput->value();
        int year = atoi(yearInput->value());

        Book newBook;
        newBook.id = libraryBooks.size() + 1;
        newBook.title = title;
        newBook.author = author;
        newBook.yearPublished = year;
        newBook.isBorrowed = false;

        libraryBooks.push_back(newBook);

        addBookWindow->hide();
        });

    addBookWindow->end();
    addBookWindow->show();
}

void rentBookCallback(Fl_Widget*, void*) {
    rentBookWindow = new Fl_Window(400, 200, "Rent a Book");

    userIdInput = new Fl_Input(150, 20, 200, 30, "User ID:");
    bookIdInput = new Fl_Input(150, 60, 200, 30, "Book ID:");

    Fl_Button* rentButton = new Fl_Button(150, 100, 100, 30, "Rent Book");
    rentButton->callback([](Fl_Widget*, void*) {
        // Get input values
        int userId = atoi(userIdInput->value());
        int bookId = atoi(bookIdInput->value());

        BorrowBook(userId, bookId, libraryUsers, libraryBooks);

        rentBookWindow->hide();
        });

    rentBookWindow->end();
    rentBookWindow->show();
}

void returnBookCallback(Fl_Widget*, void*) {
    returnBookWindow = new Fl_Window(300, 150, "Return Book");

    bookIdInput_ret = new Fl_Input(150, 20, 120, 30, "Book ID:");

    Fl_Button* returnButton = new Fl_Button(100, 70, 100, 30, "Return");
    returnButton->callback([](Fl_Widget*, void* data) {
        int bookId = atoi(bookIdInput_ret->value());

        ReturnBook(bookId, libraryUsers, libraryBooks);

        returnBookWindow->hide();
        });

    returnBookWindow->end();
    returnBookWindow->show();
}

void exitCallback(Fl_Widget*, void*) {
    ofstream UserOutputFile(usersFilePath);
    ofstream BookOutputFile(booksFilePath);
    recWriteUser(UserOutputFile, libraryUsers);
    recWriteBook(BookOutputFile, libraryBooks);
    UserOutputFile.close();
    BookOutputFile.close();

    mainWindow->hide();
	exit(0);
}

int main() {
    cout << "Import Data" << endl;
    ifstream UserInputFile(usersFilePath);
    ifstream BookInputFile(booksFilePath);
    recReadUser(UserInputFile, libraryUsers);
    recReadBook(BookInputFile, libraryBooks);
    UserInputFile.close();
    BookInputFile.close();

    cout << "FLTK INIT" <<endl;
    Fl::scheme("gtk+");

    // Create the main window
    mainWindow = new Fl_Window(550, 300, "Library Management System");

    // Create a text display widget
    textDisplay = new Fl_Text_Display(25, 10, 500, 200, "Information");
    textDisplay->buffer(new Fl_Text_Buffer());

    // Create buttons for various actions
    Fl_Button* listReadersButton = new Fl_Button(10, 220, 80, 30, "List Readers");
    listReadersButton->callback(listReadersCallback);

    Fl_Button* listBooksButton = new Fl_Button(110, 220, 80, 30, "List Books");
    listBooksButton->callback(listBooksCallback);

    Fl_Button* addReaderButton = new Fl_Button(220, 220, 100, 30, "Add Reader");
    addReaderButton->callback(addReaderCallback);

    Fl_Button* addBookButton = new Fl_Button(130, 260, 100, 30, "Add Book");
    addBookButton->callback(addBookCallback);

    Fl_Button* rentBookButton = new Fl_Button(350, 220, 80, 30, "Rent Book");
    rentBookButton->callback(rentBookCallback);

    Fl_Button* returnBookButton = new Fl_Button(460, 220, 80, 30, "Return Book");
    returnBookButton->callback(returnBookCallback);

    Fl_Button* leaveLibraryButton = new Fl_Button(235, 260, 80, 30, "Exit");
    leaveLibraryButton->callback(exitCallback);

    mainWindow->callback(exitCallback);

    mainWindow->end();
    mainWindow->show();

    return Fl::run();
}

void BorrowBook(int userId, int bookId, LibraryUsers& libraryUsers, LibraryBooks& libraryBooks) {
    auto userIt = find_if(libraryUsers.begin(), libraryUsers.end(), [userId](const User& user) {
        return user.id == userId;
        });

    if (userIt == libraryUsers.end()) {
        cout << "User not found." << endl;
        return;
    }

    auto bookIt = find_if(libraryBooks.begin(), libraryBooks.end(), [bookId](const Book& book) {
        return book.id == bookId;
        });

    if (bookIt == libraryBooks.end()) {
        cout << "Book not found." << endl;
        return;
    }

    if (find(userIt->borrowedBooksID.begin(), userIt->borrowedBooksID.end(), bookId) != userIt->borrowedBooksID.end()) {
        cout << "ERROR! Book is already borrowed by this user." << endl;
        return;
    }

    if (bookIt->isBorrowed) {
        cout << "ERROR! Book is already borrowed by another user." << endl;
        return;
    }

    bookIt->isBorrowed = true;

    userIt->borrowedBooksID.push_back(bookId);

    cout << "Book successfully borrowed by user." << endl;
}

void ReturnBook(int bookId, LibraryUsers& libraryUsers, LibraryBooks& libraryBooks) {
    auto bookIt = find_if(libraryBooks.begin(), libraryBooks.end(), [bookId](const Book& book) {
        return book.id == bookId;
        });

    if (bookIt == libraryBooks.end()) {
        cout << "Book not found." << endl;
        return;
    }

    if (!bookIt->isBorrowed) {
        cout << "Book is not currently borrowed." << endl;
        return;
    }

    bookIt->isBorrowed = false;

    for (auto& user : libraryUsers) {
        auto borrowedIt = find(user.borrowedBooksID.begin(), user.borrowedBooksID.end(), bookId);
        if (borrowedIt != user.borrowedBooksID.end()) {
            user.borrowedBooksID.erase(borrowedIt);
        }
    }

    cout << "Book successfully returned." << endl;
}
