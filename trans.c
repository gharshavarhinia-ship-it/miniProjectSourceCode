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
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
}; // end structure clientData

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
int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 11)
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
    struct clientData client = {0, "", "", 0.0};

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
        printf("%-6d%-16s%-11s%10.2f\n",
               client.acctNum,
               client.lastName,
               client.firstName,
               client.balance);
    }
}
// list all records to console
void listRecords(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int result;

    rewind(fPtr); // sets pointer to beginning of file
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    // read all records from file and display
    while (!feof(fPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, fPtr);

        // display single record if it exists
        if (result != 0 && client.acctNum != 0)
        {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                   client.balance);
        } // end if
    } // end while
} // end function listRecords
// show summary statistics
void summaryStatistics(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
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
    struct clientData client = {0, "", "", 0.0};
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
    struct clientData client = {0, "", "", 0.0};

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
    { // edit record
        printf("Current details:\n");
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // request new last name and first name
        printf("%s", "Enter new lastname, firstname\n? ");
        scanf("%14s%9s", client.lastName, client.firstName);

        printf("Updated details:\n");
        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

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
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
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
    struct clientData client = {0, "", "", 0.0};

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
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction; // update record balance
        if (client.balance < 1000)
        {
            printf("Warning Low Balance!\n");
        }
        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

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
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0}; // blank client
    unsigned int accountNum;                        // account number

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
    struct clientData client = {0, "", "", 0.0};
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
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - edit account details\n"
                 "6 - search an account\n"
                 "7 - list all accounts\n"
                 "8 - show summary statistics\n"
                 "9 - search by name\n"
                 "11 - end program\n? ");

    scanf("%u", &menuChoice); // receive choice from user
    return menuChoice;
} // end function enterChoice