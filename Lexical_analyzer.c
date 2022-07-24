#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define MAXCHAR 1000 //maxchar on a line
///////////////////////////////////////////////

int find_identifiers(char *str, int line_count); // find identifiers & make controls & create token
int find_brackets(char *str, int line_count);    // find brackets & make controls & create token
char *remove_extra_spaces(char *str);
char *strremove(char *str, const char *sub);

char *identifiers[1000];
int identifier_count = 0;
int opened_par = 0;
int opened_sq_br = 0;
int opened_curly_br = 0;
int opened_comment = 0;
int string = -1;
const char *operators[7] = {"++", "--", "*", "/", "+", "-", ":="};
const char *keywords[20] = {"break", "case", "char", "const", "continue", "do", "else", "enum", "float", "for", "goto", "if", "int", "long", "record", "return", "static", "while"};
FILE *fpo;
int main()
{
    FILE *fp;

    char str[MAXCHAR];
    char *filename = "code.Ceng";
    int line_count = 1;
    fp = fopen(filename, "r"); // start to read file

    fpo = fopen("code.lex", "w");
    if (fp == NULL)
    {
        printf("Could not open file %s", filename);
        return 1;
    }
    while (fgets(str, MAXCHAR, fp) != NULL)
    { // read file line by line

        find_identifiers(str, line_count);
        line_count++;
    }

    if (opened_par > 0)
    {
        fprintf(fpo, "Error! Expected ')'\n");
        exit(0);
    }
    else if (opened_sq_br > 0)
    {
        fprintf(fpo, "Error! Expected ']'\n");
        exit(0);
    }

    else if (opened_curly_br > 0)
    {
        fprintf(fpo, "Error! Expected '}'\n");
        exit(0);
    }

    else if (opened_comment > 0)
    {
        fprintf(fpo, "Error! Expected '*)'\n");
        exit(0);
    }
    else if (string == 0)
    {
        fprintf(fpo, "Error! Expected '\"'\n");
        exit(0);
    }

    fprintf(fpo, "\n");
    fclose(fp);
    return 0;
}

int find_identifiers(char *str, int line_count)
{
    int can_be_identifier = 0;
    char string_buffer[1000];
    int stringcounter = 0;
    int keyword = 0;
    int s = -1;
    char double_quote_buffer[1000];
    int dqb_counter = 0;
    int comment = 0;
    char *numbuffer[1000];
    int numcounter = 0;

    str = remove_extra_spaces(str);
    for (int i = 0; i < strlen(str); i++)
    {
        // printf("char %c\n", str[i]);
        if (str[i] == '\"')
        {
            s++;
            if (s == 1)
            {
                s = -1;
                fprintf(fpo, "StringConst(%s)\n", double_quote_buffer);
                for (int i = 0; i < dqb_counter; i++)
                {
                    double_quote_buffer[dqb_counter] = '\0';
                }
                dqb_counter = 0;
            }
        }
        else if (s > -1)
        {
            double_quote_buffer[dqb_counter] = str[i];
            dqb_counter++;
        }

        if (isalpha(str[i]) && s == -1 && comment == 0 && opened_comment == 0) //it can be identifier
        {

            can_be_identifier = 1;
        }
        if (can_be_identifier == 1)
        {

            if (isalnum(str[i]) || str[i] == '_')
            {

                string_buffer[stringcounter] = str[i];
                stringcounter++;
            }
            else
            {

                can_be_identifier = 0;
                string_buffer[stringcounter] = '\0';

                for (int i = 0; i < 17; i++)
                {
                    if (strcmp(string_buffer, keywords[i]) == 0 && opened_comment == 0)
                    {
                        fprintf(fpo, "Keyword(%s)\n", string_buffer);
                        keyword = 1;
                    }
                }
                if (keyword == 0 && opened_comment == 0)
                {
                    if (stringcounter < 21)
                    {
                        fprintf(fpo, "Identifier(%s)\n", string_buffer);
                    }
                    else
                    {
                        fprintf(fpo, "Error! Identifiers' maximum size is 20 ");
                        exit(0);
                    }
                }
                keyword = 0;
                for (int i = 0; i < stringcounter; i++)
                {
                    string_buffer[i] = '\0';
                }
                stringcounter = 0;
            }
        }
        if (str[i] == '(' && str[i + 1] == '*')
        {
            comment = 1;
            opened_comment++;
        }
        if (comment == 0 && opened_comment == 0)
        {
            if (str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/' || str[i] == ':')
            {
                if (str[i + 1] == '+' || str[i + 1] == '-' || str[i + 1] == '=')
                {
                    fprintf(fpo, "Operator(%c%c)\n", str[i], str[i + 1]);
                }
                else if (str[i - 1] == '+' || str[i - 1] == '-' || str[i - 1] == ':')
                {
                    if (str[i + 1] == ';')
                        fprintf(fpo, "EndOfLine");
                    break;
                }

                else
                {
                    fprintf(fpo, "Operator(%c)\n", str[i]);
                }
            }
        }
        if (str[i] == '*' && str[i + 1] == ')')
        {
            comment = 0;
            opened_comment--;
        }

        if (isdigit(str[i]) && can_be_identifier == 0)
        {

            numbuffer[numcounter] = str[i];

            numcounter++;
        }
        if (str[i] == ';')
        {
            if (numcounter > 0 && opened_comment == 0)
            {
                if (numcounter < 11)
                {
                    numbuffer[numcounter] = '\0';
                    fprintf(fpo, "IntConst(%d)\n", atoi(numbuffer));
                }
                else
                {
                    fprintf(fpo, "Integers' maximum size is 10.");
                    exit(0);
                }
                for (int i = 0; i < numcounter + 1; i++)
                {
                    numbuffer[i] = '\0';
                }
                numcounter = 0;
            }
            fprintf(fpo, "EndOfLine\n");
        }
    }

    str = strremove(str, "(*");
    str = strremove(str, "*)");
    find_brackets(str, line_count);

    return 0;
}

int find_brackets(char *str, int line_count)
{
    /*
        LeftPar: ( RightPar: )
        LeftSquareBracket: [ RightSquareBracket: ]
        LeftCurlyBracket: { RightCurlyBracket: }
        Example Token: LeftCurlyBracket 
    */

    if (strstr(str, "(") != NULL)
    {
        fprintf(fpo, "LeftPar\n");
        opened_par++;
    }

    if (strstr(str, ")") != NULL)
    {
        fprintf(fpo, "RightPar\n");
        opened_par--;
    }

    if (strstr(str, "[") != NULL)
    {
        fprintf(fpo, "LeftSquareBracket\n");
        opened_sq_br++;
    }

    if (strstr(str, "]") != NULL)
    {
        fprintf(fpo, "RightSquareBracket\n");
        opened_sq_br--;
    }

    if (strstr(str, "{") != NULL)
    {
        if (opened_par == 0)
        {
            fprintf(fpo, "LeftCurlyBracket\n");
            opened_curly_br++;
        }
        else
        {
            fprintf(fpo, "Error! Expected ')'");
            exit(0);
        }
    }

    if (strstr(str, "}") != NULL)
    {
        fprintf(fpo, "RightCurlyBracket\n");
        opened_curly_br--;
    }
}

char *remove_extra_spaces(char *str)
{
    int i, x;
    for (i = x = 0; str[i]; ++i)
    {
        if (!isspace(str[i]) || (i > 0 && !isspace(str[i - 1])))
            str[x++] = str[i];
    }

    str[x] = '\0';
    return str;
}

char *strremove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if (len > 0)
    {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL)
        {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}