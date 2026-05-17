
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct clientData
{
    unsigned int acctNum; 
    unsigned int slot;    
    char lastName[15];    
    char firstName[10];   
    double balance;      
}; 


struct clientDataOld
{
    unsigned int acctNum; 
    char lastName[15];   
    char firstName[10];   
    double balance;       
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

    
    fseek(fPtr, (fromAcc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&fromClient, sizeof(struct clientData), 1, fPtr);

    fseek(fPtr, (toAcc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&toClient, sizeof(struct clientData), 1, fPtr);

   
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

    
    fromClient.balance -= amount;
    toClient.balance += amount;

    
    fseek(fPtr, (fromAcc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&fromClient, sizeof(struct clientData), 1, fPtr);

    
    fseek(fPtr, (toAcc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&toClient, sizeof(struct clientData), 1, fPtr);

    printf("\nTransfer Successful!\n");

    printf("From Account (%u) New Balance: %.2f\n", fromAcc, fromClient.balance);
    printf("To Account (%u) New Balance: %.2f\n", toAcc, toClient.balance);
}

void migrateFileIfNeeded(FILE **fPtr, const char *filename)
{
    long oldRecSize = sizeof(struct clientDataOld);
    long newRecSize = sizeof(struct clientData);

    fseek(*fPtr, 0, SEEK_END);
    long fileSize = ftell(*fPtr);

   
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
    FILE *cfPtr;         
    unsigned int choice; 

    cfPtr = openCreditFile("credit.dat");
    if (cfPtr == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    while ((choice = enterChoice()) != 12)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
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
        case 12:
            puts("Invalid options. Please enter a number between 1 and 12.");
            break;
        
        } 
    } 

    fclose(cfPtr);
} 
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
void listRecords(FILE *fPtr)
{
    struct clientData client = {0, 0, "", "", 0.0};
    int result;

    rewind(fPtr); 
    printf("%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");

    
    while (!feof(fPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, fPtr);

    
        if (result != 0 && client.acctNum != 0)
        {
            printf("%-8u%-6u%-16s%-11s%10.2f\n", client.acctNum, client.slot, client.lastName, client.firstName,
                   client.balance);
        }
    } 
} 

void summaryStatistics(FILE *fPtr)
{
    struct clientData client = {0, 0, "", "", 0.0};
    int result;
    int count = 0;
    double totalBalance = 0.0;
    double minBalance = 0.0;
    double maxBalance = 0.0;

    rewind(fPtr); 

   
    while (!feof(fPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, fPtr);

        
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
        } 
    }
    printf("\nSummary Statistics:\n");
    printf("Total Accounts: %d\n", count);
    if (count > 0)
    {
        printf("Total Balance: %.2f\n", totalBalance);
        printf("Average Balance: %.2f\n", totalBalance / count);
        printf("Minimum Balance: %.2f\n", minBalance);
        printf("Maximum Balance: %.2f\n", maxBalance);
    }
} 
void searchByName(FILE *fPtr)
{
    struct clientData client = {0, 0, "", "", 0.0};
    int result;
    char searchName[15];
    int found = 0;

    printf("Enter last name to search: ");
    scanf("%14s", searchName);

    rewind(fPtr); 
    printf("\nAccounts with last name '%s':\n", searchName);
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    while (!feof(fPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, fPtr);


        if (result != 0 && client.acctNum != 0)
        {
            if (strcmp(client.lastName, searchName) == 0)
            {
                printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                       client.balance);
                found = 1;
            }
        } 
    } 

    if (!found)
    {
        printf("No accounts found with last name '%s'.\n", searchName);
    }
} 

void editAccount(FILE *fPtr)
{
    unsigned int account; 
    struct clientData client = {0, 0, "", "", 0.0};

    
    printf("%s", "Enter account to edit ( 1 - 100 ): ");
    scanf("%d", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    {                          
        client.slot = account; 

        printf("Current details:\n");
        printf("%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");
        printf("%-8u%-6u%-16s%-11s%10.2f\n\n", client.acctNum, client.slot, client.lastName, client.firstName, client.balance);
        printf("%s", "Enter new lastname, firstname\n? ");
        scanf("%14s%9s", client.lastName, client.firstName);

        printf("Updated details:\n");
        printf("%-8u%-6u%-16s%-11s%10.2f\n", client.acctNum, client.slot, client.lastName, client.firstName, client.balance);

        
        fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
        
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
} 

void textFile(FILE *readPtr)
{
    FILE *writePtr; 
    
    int result;     
    
    
    
    struct clientData client = {0, 0, "", "", 0.0};

   
    
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } 
    else
    {
        rewind(readPtr);
        
        fprintf(writePtr, "%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");

       
        
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

           
            
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-8u%-6u%-16s%-11s%10.2f\n", client.acctNum, client.slot, client.lastName, client.firstName,
                        client.balance);
            } 
        } 

        fclose(writePtr); 
        
    } 
} 


void updateRecord(FILE *fPtr)
{
    unsigned int account; 
    double transaction;   
   
    struct clientData client = {0, 0, "", "", 0.0};

    
    
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);

    
    
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
   
    
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    {                          
        
        client.slot = account; 
        
        printf("%-8s%-6s%-16s%-11s%10s\n", "Acct", "Slot", "Last Name", "First Name", "Balance");
        printf("%-8u%-6u%-16s%-11s%10.2f\n\n", client.acctNum, client.slot, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction; 
        
        if (client.balance < 1000)
        {
            printf("Warning Low Balance!\n");
        }
        printf("%-8u%-6u%-16s%-11s%10.2f\n", client.acctNum, client.slot, client.lastName, client.firstName, client.balance);

        
        
        fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
       
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } 
}

void deleteRecord(FILE *fPtr)
{
    struct clientData client;                            
    
    struct clientData blankClient = {0, 0, "", "", 0.0}; 
    unsigned int accountNum;                            

   
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);

   
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    }
    else
    { 
        blankClient.slot = accountNum;
        
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    } 
} 


void newRecord(FILE *fPtr)
{
   
    struct clientData client = {0, 0, "", "", 0.0};
    unsigned int accountNum; // account number

    
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);

   
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
   
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    }
    else
    { 
       
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);

        client.acctNum = accountNum;
        client.slot = accountNum;
        
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } 
} 


unsigned int enterChoice(void)
{
    unsigned int menuChoice; 
    

    printf("%s",
                 "\n-------------------------------------------"
                 "\n|           BANK MANAGEMENT                |"
                 "\n|          Enter your choice               |"
                 "\n-------------------------------------------\n"
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

    scanf("%u", &menuChoice); 
    return menuChoice;
} 