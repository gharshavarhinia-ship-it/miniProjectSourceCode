// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    unsigned int slot;    // record slot number (1-based)
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
}; // end structure clientData

// old format used before slot field was added
struct clientDataOld
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};

#define MAX_RECORDS 100

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void searchRecord(FILE *fPtr);
void listRecords(FILE *fPtr);
void summaryStatistics(FILE *fPtr);
void searchByName(FILE *fPtr);
void editAccount(FILE *fPtr);
void sortRecords(FILE *fPtr);
void transferMoney(FILE *fPtr);
FILE *openCreditFile(const char *filename);
void migrateFileIfNeeded(FILE **fPtr, const char *filename);

// open the credit file, create if missing, and ensure it uses the current record format
FILE *openCreditFile(const char *filename)
{
    FILE *fPtr = fopen(filename, "rb+");
    if (fPtr == NULL)
    {
        fPtr = fopen(filename, "wb+");
        if (fPtr == NULL)
            return NULL;
    }

    migrateFileIfNeeded(&fPtr, filename);
    return fPtr;
}
void transferMoney(FILE *fPtr)
{
    unsigned int fromAcc, toAcc;
    double amount;

    struct clientData fromClient = {0, 0, "", "", 0.0};
    struct clientData toClient = {0, 0, "", "", 0.0};

    printf("Enter FROM account number: ");
    scanf("%u", &fromAcc);

    printf("Enter TO account number: ");
    scanf("%u", &toAcc);

    printf("Enter amount to transfer: ");
    scanf("%lf", &amount);

    // read sender
    fseek(fPtr, (fromAcc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&fromClient, sizeof(struct clientData), 1, fPtr);

    // read receiver
    fseek(fPtr, (toAcc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&toClient, sizeof(struct clientData), 1, fPtr);

    // validation
    if (fromClient.acctNum == 0 || toClient.acctNum == 0)
    {
        printf("Invalid account number(s).\n");
        return;
    }

    if (amount <= 0)
    {
        printf("Invalid amount.\n");
        return;
    }

    if (fromClient.balance < amount)
    {
        printf("Insufficient balance in account %u.\n", fromAcc);
        return;
    }

    // perform transfer
    fromClient.balance -= amount;
    toClient.balance += amount;

    // write sender back
    fseek(fPtr, (fromAcc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&fromClient, sizeof(struct clientData), 1, fPtr);

    // write receiver back
    fseek(fPtr, (toAcc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&toClient, sizeof(struct clientData), 1, fPtr);

    printf("\nTransfer Successful!\n");

    printf("From Account (%u) New Balance: %.2f\n", fromAcc, fromClient.balance);
    printf("To Account (%u) New Balance: %.2f\n", toAcc, toClient.balance);
}
// Ensure the file uses the current struct format (with slot field).
// If the file is in the old format (no slot), rewrite it to the new format.
void migrateFileIfNeeded(FILE **fPtr, const char *filename)
{
    long oldRecSize = sizeof(struct clientDataOld);
    long newRecSize = sizeof(struct clientData);

    // determine file size
    fseek(*fPtr, 0, SEEK_END);
    long fileSize = ftell(*fPtr);

    // if file is empty, create an empty file with correct format
    if (fileSize == 0)
    {
        rewind(*fPtr);
        struct clientData blank = {0, 0, "", "", 0.0};
        for (int i = 0; i < MAX_RECORDS; i++)
        {
            blank.slot = i + 1;
            fwrite(&blank, newRecSize, 1, *fPtr);
        }
        fflush(*fPtr);
        rewind(*fPtr);
        return;
    }

    // if already in new format, ensure slots are correct and return
    if (fileSize == newRecSize * MAX_RECORDS)
    {
        rewind(*fPtr);
        for (int i = 0; i < MAX_RECORDS; i++)
        {
            struct clientData client;
            if (fread(&client, newRecSize, 1, *fPtr) != 1)
                break;
            if (client.acctNum != 0 && client.slot != (unsigned int)(i + 1))
            {
                client.slot = i + 1;
                fseek(*fPtr, -newRecSize, SEEK_CUR);
                fwrite(&client, newRecSize, 1, *fPtr);
                fflush(*fPtr);
                fseek(*fPtr, (i + 1) * newRecSize, SEEK_SET);
            }
        }
        rewind(*fPtr);
        return;
    }

    // otherwise assume old format (or truncated file) and migrate
    {
        FILE *tmp = fopen("credit.tmp", "wb");
        if (tmp == NULL)
            return;

        rewind(*fPtr);
        for (int i = 0; i < MAX_RECORDS; i++)
        {
            struct clientData client = {0, 0, "", "", 0.0};
            if (i < (fileSize / oldRecSize))
            {
                struct clientDataOld old;
                if (fread(&old, oldRecSize, 1, *fPtr) == 1)
                {
                    client.acctNum = old.acctNum;
                    client.balance = old.balance;
                    strncpy(client.lastName, old.lastName, sizeof(client.lastName));
                    client.lastName[sizeof(client.lastName) - 1] = '\0';
                    strncpy(client.firstName, old.firstName, sizeof(client.firstName));
                    client.firstName[sizeof(client.firstName) - 1] = '\0';
                }
            }

            client.slot = i + 1;
            fwrite(&client, newRecSize, 1, tmp);
        }

        fclose(*fPtr);
        fclose(tmp);
        remove(filename);
        rename("credit.tmp", filename);
        *fPtr = fopen(filename, "rb+");
    }
}

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // open and migrate file format if needed
    cfPtr = openCreditFile("credit.dat");
    if (cfPtr == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 12)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        case 6:
            searchRecord(cfPtr);
            break;
        case 7:
            listRecords(cfPtr);
            break;
        case 8:
            summaryStatistics(cfPtr);
            break;
        case 9:
            searchByName(cfPtr);
            break;
        case 5:
            editAccount(cfPtr);
            break;
        case 10:
            sortRecords(cfPtr);
            break;
        case 11:
            transferMoney(cfPtr);
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    } // end while

    fclose(cfPtr); // fclose closes the file
} // end main
// search
void searchRecord(FILE *fPtr)
{
    unsigned int account;
    struct clientData client = {0, 0, "", "", 0.0};

    printf("Enter account number to search: ");
    scanf("%u", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account not found.\n");
    }
    else
    {
        printf("Account Found:\n");
        printf("%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");
        printf("%-8u%-6u%-16s%-11s%10.2f\n",
               client.acctNum,
               client.slot,
               client.lastName,
               client.firstName,
               client.balance);
    }
}
void sortRecords(FILE *fPtr)
{
    struct clientData clients[MAX_RECORDS];
    int count = 0;

    rewind(fPtr);

    // read all valid records
    while (fread(&clients[count], sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (clients[count].acctNum != 0)
            count++;
    }

    if (count == 0)
    {
        printf("No records to display.\n");
        return;
    }

    // sort by balance (high → low)
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (clients[j].balance < clients[j + 1].balance)
            {
                struct clientData temp = clients[j];
                clients[j] = clients[j + 1];
                clients[j + 1] = temp;
            }
        }
    }

    // display sorted records
    printf("\nAccounts sorted by balance (High → Low):\n");
    printf("%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");

    for (int i = 0; i < count; i++)
    {
        printf("%-8u%-6u%-16s%-11s%10.2f\n",
               clients[i].acctNum,
               clients[i].slot,
               clients[i].lastName,
               clients[i].firstName,
               clients[i].balance);
    }
}
// list all records to console
void listRecords(FILE *fPtr)
{
    struct clientData client = {0, 0, "", "", 0.0};
    int result;

    rewind(fPtr); // sets pointer to beginning of file
    printf("%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");

    // read all records from file and display
    while (!feof(fPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, fPtr);

        // display single record if it exists
        if (result != 0 && client.acctNum != 0)
        {
            printf("%-8u%-6u%-16s%-11s%10.2f\n", client.acctNum, client.slot, client.lastName, client.firstName,
                   client.balance);
        } // end if
    } // end while
} // end function listRecords
// show summary statistics
void summaryStatistics(FILE *fPtr)
{
    struct clientData client = {0, 0, "", "", 0.0};
    int result;
    int count = 0;
    double totalBalance = 0.0;
    double minBalance = 0.0;
    double maxBalance = 0.0;

    rewind(fPtr); // sets pointer to beginning of file

    // read all records from file
    while (!feof(fPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, fPtr);

        // process single record if it exists
        if (result != 0 && client.acctNum != 0)
        {
            count++;
            totalBalance += client.balance;
            if (count == 1)
            {
                minBalance = maxBalance = client.balance;
            }
            else
            {
                if (client.balance < minBalance)
                    minBalance = client.balance;
                if (client.balance > maxBalance)
                    maxBalance = client.balance;
            }
        } // end if
    } // end while

    printf("\nSummary Statistics:\n");
    printf("Total Accounts: %d\n", count);
    if (count > 0)
    {
        printf("Total Balance: %.2f\n", totalBalance);
        printf("Average Balance: %.2f\n", totalBalance / count);
        printf("Minimum Balance: %.2f\n", minBalance);
        printf("Maximum Balance: %.2f\n", maxBalance);
    }
} // end function summaryStatistics
// search by name
void searchByName(FILE *fPtr)
{
    struct clientData client = {0, 0, "", "", 0.0};
    int result;
    char searchName[15];
    int found = 0;

    printf("Enter last name to search: ");
    scanf("%14s", searchName);

    rewind(fPtr); // sets pointer to beginning of file
    printf("\nAccounts with last name '%s':\n", searchName);
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    // read all records from file
    while (!feof(fPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, fPtr);

        // check single record if it exists
        if (result != 0 && client.acctNum != 0)
        {
            if (strcmp(client.lastName, searchName) == 0)
            {
                printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                       client.balance);
                found = 1;
            }
        } // end if
    } // end while

    if (!found)
    {
        printf("No accounts found with last name '%s'.\n", searchName);
    }
} // end function searchByName
// edit account details
void editAccount(FILE *fPtr)
{
    unsigned int account; // account number
    struct clientData client = {0, 0, "", "", 0.0};

    // obtain number of account to edit
    printf("%s", "Enter account to edit ( 1 - 100 ): ");
    scanf("%d", &account);

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    {                          // edit record
        client.slot = account; // ensure slot is correct

        printf("Current details:\n");
        printf("%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");
        printf("%-8u%-6u%-16s%-11s%10.2f\n\n", client.acctNum, client.slot, client.lastName, client.firstName, client.balance);

        // request new last name and first name
        printf("%s", "Enter new lastname, firstname\n? ");
        scanf("%14s%9s", client.lastName, client.firstName);

        printf("Updated details:\n");
        printf("%-8u%-6u%-16s%-11s%10.2f\n", client.acctNum, client.slot, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
} // end function editAccount
// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, 0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-8u%-6u%-16s%-11s%10.2f\n", client.acctNum, client.slot, client.lastName, client.firstName,
                        client.balance);
            } // end if
        } // end while

        fclose(writePtr); // fclose closes the file
    } // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    // create clientData with no information
    struct clientData client = {0, 0, "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    {                          // update record
        client.slot = account; // ensure slot is correct
        printf("%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");
        printf("%-8u%-6u%-16s%-11s%10.2f\n\n", client.acctNum, client.slot, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction; // update record balance
        if (client.balance < 1000)
        {
            printf("Warning Low Balance!\n");
        }
        printf("%-8u%-6u%-16s%-11s%10.2f\n", client.acctNum, client.slot, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        // move back by 1 record length
        fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                            // stores record read from file
    struct clientData blankClient = {0, 0, "", "", 0.0}; // blank client
    unsigned int accountNum;                             // account number

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        blankClient.slot = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, 0, "", "", 0.0};
    unsigned int accountNum; // account number

    // obtain number of account to create
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);

        client.acctNum = accountNum;
        client.slot = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function newRecord

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 -  store a formatted text file (accounts.txt)\n"
                 "2 -  update an account\n"
                 "3 -  add a new account\n"
                 "4 -  delete an account\n"
                 "5 -  edit account details\n"
                 "6 -  search an account\n"
                 "7 -  list all accounts\n"
                 "8 -  show summary statistics\n"
                 "9 -  search by name\n"
                 "10 - sort accounts by balance\n"
                 "11 - transfer money between accounts\n"
                 "12 - end program\n? ");

    scanf("%u", &menuChoice); // receive choice from user
    return menuChoice;
} // end function enterChoice