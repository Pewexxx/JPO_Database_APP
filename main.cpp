#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <algorithm>

#include <FL/FL.h>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>

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
    vector<Book> borrowedBooks;
};

using LibraryUsers = std::vector<User>;
using LibraryBooks = std::vector<Book>;


ostream& operator<<(ostream& o, const Address& address) {
    o << address.city << ' ' << address.street << ' ' << address.house << ' ' << address.flat;
    return o;
}

istream& operator>>(istream& i, Address& address) {
    i >> address.city >> address.street >> address.house >> address.flat;
    return i;
}

ostream& operator<<(ostream& o, const Book& book) {
    o << book.id << ' ' << book.title << ' ' << book.author << ' ' << book.yearPublished << ' ' << book.isBorrowed;
    return o;
}

istream& operator>>(istream& i, Book& book) {
    i >> book.id >> book.title >> book.author >> book.yearPublished >> book.isBorrowed;
    return i;
}

ostream& operator<<(ostream& o, const User& user) {
    o << user.id << ' ' << user.forename << ' ' << user.surname << ' ' << user.address << '\n';

    for (const auto& borrowedBook : user.borrowedBooks) {
        o << borrowedBook << '\n';
    }

    return o;
}

istream& operator>>(istream& i, User& user) {
    i >> user.id >> user.forename >> user.surname >> user.address;

    Book borrowedBook;
    while (i >> borrowedBook) {
        user.borrowedBooks.push_back(borrowedBook);
    }

    return i;
}


void FillLibraryUser(LibraryUsers& libraryUsers);

void ExportData(LibraryUsers& libraryUsers, LibraryBooks& libraryBooks);

void BorrowBook(int userId, int bookId, LibraryUsers& libraryUsers, LibraryBooks& libraryBooks);


int main() {
    LibraryUsers libraryUsers;
    LibraryBooks libraryBooks;
    int borrow_BookId = 0;
    int borrow_UserId = 0;

    FillLibraryUser(libraryUsers);

    BorrowBook(borrow_UserId, borrow_BookId, libraryUsers, libraryBooks);

    ExportData(libraryUsers, libraryBooks);
    return 0;
}

void ExportData(LibraryUsers& libraryUsers, LibraryBooks& libraryBooks) {
    ofstream outputUser("LibraryUsers_Data.txt");
    ofstream outputBook("LibraryBooks_Data.txt");

    copy(libraryUsers.begin(), libraryUsers.end(), ostream_iterator<User>(outputUser, "\n"));
    copy(libraryBooks.begin(), libraryBooks.end(), ostream_iterator<Book>(outputBook, "\n"));

    outputBook.close();
    outputUser.close();
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

    if (bookIt->isBorrowed) {
        cout << "ERROR! Book is already borrowed by another user." << endl;
        return;
    }

    userIt->borrowedBooks.push_back(*bookIt);
    bookIt->isBorrowed = true;

    cout << "Book successfully borrowed by user." << endl;
}

void FillLibraryUser(LibraryUsers& libraryUsers) {
    User user{};

    user.id = libraryUsers.size() + 1;
    cout << "Enter Forename: ";
    cin >> user.forename;
    cout << "Enter Surname: ";
    cin >> user.surname;
    cout << "Enter city: ";
    cin >> user.address.city;
    cout << "Enter street: ";
    cin >> user.address.street;
    cout << "Enter house: ";
    cin >> user.address.house;
    cout << "Enter flat: ";
    cin.ignore();
    getline(cin, user.address.flat);

    libraryUsers.push_back(user);
}