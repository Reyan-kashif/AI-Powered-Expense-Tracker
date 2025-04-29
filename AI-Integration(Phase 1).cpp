/*1. Install OpenSSL 
I used this link: https://www.hostnoc.com/install-openssl-on-windows/
2. Unzip the folder this program is in on your system and open the "vs code" folder
3. In tasks.json, set the file path to where you saved the folder.
4. Under "args" in the same file, add these 2 lines:  
"-lssl", 
"-lcrypto"
5. In launch.json, paste the same path after "program:"*/

#include <iostream>
#include <fstream>
#include <string>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
using namespace std;

//key for aes-256
//aes-256-> 256 bits so 32 bytes
//each character is 1 byte which equates to 32 characters 
const string AES_KEY = "ThisIsA32ByteLongSecretKeyForAES!"; //has to be exact 32 characters

// derive 16-byte iv from a string
void derive_iv(const string& source, unsigned char* iv) {
    //makes an array "hash" of size 32 (-> sha-256 again so 32 bytes)
    //type "unsigned" because a hash can't have negative numbers so we have the full range of values(0-255)
    unsigned char hash[32];
    //OPENSSL built in method 
    //1. passing pointer to first char of string, 
    //2. the length so it knows how many bytes to read and 
    //3. the hash array so the function writes the output into it
    SHA256((unsigned char*)source.c_str(), source.length(), hash);
    memcpy(iv, hash, 16); // use first 16 bytes of SHA-256 as IV
}

// hash a password using sha256
string sha256(const string& input) {
    unsigned char hash[32];
    SHA256((unsigned char*)input.c_str(), input.length(), hash);

    //like cin, cout
    //cleaner alternative to concatenation
    stringstream ss;
    //1 hex character represents 4 bits so we need 2 hex characters to form 1 byte
    //and hence 64 characters for the full hash
    //this loop converts each hexa into a two character version and if there is no preceding value it sets it to zero
    for (int i = 0; i < 32; i++)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

// encrypt using aes-256-cbc
string aes256_encrypt(const string& plaintext, const string& key, const string& inputForIV) {
    unsigned char iv[16];
    derive_iv(inputForIV, iv);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new(); // create an "encryption context"

    if (!ctx) {
        cout<<"error creating encryption context";
        exit(1);
    }
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char*)key.c_str(), iv))
        exit(1);

    // padding is the process of adding extra bytes to make the text a multiple of the block size 
    // this ensures the data can be properly encrypted and decrypted
    // padding the input text to make it fit the required block size (that is 16 bytes in AES' case)
    // padding ensures the text length is a multiple of 16 for the encryption to work
    int padding = 16 - (plaintext.length() % 16);
    int totalLength = plaintext.length() + padding;
    unsigned char* padded = new unsigned char[totalLength];
    memcpy(padded, plaintext.c_str(), plaintext.length());
    memset(padded + plaintext.length(), padding, padding);

    unsigned char* ciphertext = new unsigned char[totalLength + 16]; // to allow for extra padding
    int len, ciphertext_len;

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, padded, totalLength))
        exit(1);
    ciphertext_len = len;

    // finalize the encryption process
    // ensures the final part of the text is encrypted
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        exit(1);
    ciphertext_len += len;

    stringstream ss;
    for (int i = 0; i < ciphertext_len; i++)
        ss << hex << setw(2) << setfill('0') << (int)ciphertext[i];

    // free memory properly
    delete[] padded;
    delete[] ciphertext;
    EVP_CIPHER_CTX_free(ctx);

    return ss.str();
}

//  User class to initialise credentials
class User {
public:
    string username; // encrypted username in hexa form
    string email; //""
    string passwordHash; //hashed password

    //initialise manually
    User(string un, string mail, string hashPwd) {
        username = un;
        email = mail;
        passwordHash = hashPwd;
    }
};

// The Authentication System class (our main functioning class)
class AuthenSystem {
public:
    void signup() {
        string un, mail, pwd;
        cout << "Enter username: "; 
        cin >> un;
        cout << "Enter email: "; 
        cin >> mail;
        cout << "Enter password: ";
        cin >> pwd;

        // store hashed password, and encrypted data by calling earlier self-made functions
        string hashedPwd = sha256(pwd);
        string encryptedUn = aes256_encrypt(un, AES_KEY, un);
        string encryptedMail = aes256_encrypt(mail, AES_KEY, mail);

        // make the User instance and pass prepared parameters into it
        User newUser(encryptedUn, encryptedMail, hashedPwd);

        // Open the file to store the user info in write mode
        //ios::app will make the file and write to it on first run
        ofstream file("users.txt", ios::app);
        if (file.is_open()) {
            file << newUser.username << " " << newUser.email << " " << newUser.passwordHash << endl;
            file.close();
            cout << "Signup successful!" << endl;
        } else {
            cerr << "Failed to open file!" << endl;
        }
    }

    void login() {
        //take user input once again to match with stored credentials
        string un, mail, pwd;
        cout << "Enter username: ";
        cin >> un;
        cout << "Enter email: ";
        cin >> mail;
        cout << "Enter password: ";
        cin >> pwd;

        string encryptedUn = aes256_encrypt(un, AES_KEY, un);
        string encryptedMail = aes256_encrypt(mail, AES_KEY, mail);
        //since we dont want to decrypt the hash revealing our password we can just get the hash 
        //for the entered password and compare that as the same password will also have the same hash
        string hashedPwd = sha256(pwd);

        //open file in read mode for comparison
        ifstream file("users.txt");
        //prepare variable to store file contents
        string file_un, file_mail, file_hash;
        //initialise a verification flag with false
        bool found = false;

        //read and compare contents untill eof
        while (file >> file_un >> file_mail >> file_hash) {
            if (file_un == encryptedUn && file_mail == encryptedMail && file_hash == hashedPwd) {
                found = true;
                break;
            }
        }

        file.close();

        if (found) {
            cout << "Login successful!" << endl;
        } else {
            cout << "Invalid credentials!" << endl;
        }
    }
};

// main menu
// uses object to access methods
int main() {
    AuthenSystem auth;
    int choice;
    do {
        cout << "\n1. Signup\n2. Login\n0. Exit\nEnter choice: ";
        cin >> choice;
        switch (choice) {
            case 1: auth.signup(); break;
            case 2: auth.login(); break;
        }
    } while (choice != 0); //set base case to terminate
}





