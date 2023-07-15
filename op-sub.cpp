/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "op-sub.hpp"

/*
Order of operations for computers:

Brackets
Comparison
And / Or / Negation
Mult / Div / Mod
Add / Sub
Bitwise
*/

/*
Without ordered operations:

1 < 2 && 3 > 4
Less(1, 2) && 3 > 4
And(Less(1, 2), 3) > 4
Less(And(Less(1, 2), 3), 4)
(1 < 2 && 3) > 4

With ordered operations:

1 < 2 && 3 > 4
Less(1, 2) && Great(3, 4)
And(Less(1, 2), Great(3, 4))
*/

void doSub(vector<string> &From, const int &i, const string &SubName, const bool &IsConst)
{
    // Everything before, after, and what to replace all this with
    vector<string> prevGroup, postGroup, toAdd;

    // Construct groups
    for (int j = 0; j < i; j++)
    {
        prevGroup.push_back(From[j]);
    }
    for (int j = i + 1; j < From.size(); j++)
    {
        postGroup.push_back(From[j]);
    }

    // Do operator substitutions on both sides
    operatorSub(prevGroup);
    operatorSub(postGroup);

    // Reconstruct
    toAdd.push_back(SubName);
    toAdd.push_back("(");
    if (!IsConst)
    {
        toAdd.push_back("&");
    }
    toAdd.push_back("(");

    for (auto s : prevGroup)
    {
        toAdd.push_back(s);
    }

    toAdd.push_back(")");
    toAdd.push_back(",");
    toAdd.push_back("(");

    for (auto s : postGroup)
    {
        toAdd.push_back(s);
    }

    toAdd.push_back(")");
    toAdd.push_back(")");

    From = toAdd;
    return;
}

void operatorSub(vector<string> &From)
{
    // Level 1: Assignment
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "=")
        {
            doSub(From, i, "Copy", false);
            break;
        }
        else if (cur == "+=")
        {
            doSub(From, i, "AddEq", false);
            break;
        }
        else if (cur == "-=")
        {
            doSub(From, i, "SubEq", false);
            break;
        }
        else if (cur == "*=")
        {
            doSub(From, i, "MultEq", false);
            break;
        }
        else if (cur == "/=")
        {
            doSub(From, i, "DivEq", false);
            break;
        }
        else if (cur == "%=")
        {
            doSub(From, i, "ModEq", false);
            break;
        }
        else if (cur == "^=")
        {
            doSub(From, i, "XorEq", false);
            break;
        }
        else if (cur == "++")
        {
            doSub(From, i, "Incr", false);
            break;
        }
        else if (cur == "--")
        {
            doSub(From, i, "Decr", false);
            break;
        }
        else if (cur == "&=")
        {
            doSub(From, i, "AndEq", false);
            break;
        }
        else if (cur == "|=")
        {
            doSub(From, i, "OrEq", false);
            break;
        }
    }

    // Level 2: Containers
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "[")
        {
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            assert(false);
        }
    }

    // Level 3: Comparisons
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "<")
        {
            doSub(From, i, "Less", true);
            break;
        }
        else if (cur == ">")
        {
            doSub(From, i, "Great", true);
            break;
        }
        else if (cur == "<=")
        {
            doSub(From, i, "Leq", true);
            break;
        }
        else if (cur == ">=")
        {
            doSub(From, i, "Greq", true);
            break;
        }
        else if (cur == "==")
        {
            doSub(From, i, "Eq", true);
            break;
        }
        else if (cur == "!=")
        {
            doSub(From, i, "Neq", true);
            break;
        }
    }

    // Level 4: Booleans
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "!")
        {
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
            assert(false);
        }
        else if (cur == "&&")
        {
            doSub(From, i, "Andd", true);
            break;
        }
        else if (cur == "||")
        {
            doSub(From, i, "Orr", true);
            break;
        }
    }

    // Level 5: Multiplication, division and modulo
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "*")
        {
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            doSub(From, i, "Mult", true);
            break;
        }
        else if (cur == "/")
        {
            doSub(From, i, "Div", true);
            break;
        }
        else if (cur == "%")
        {
            doSub(From, i, "Mod", true);
            break;
        }
    }

    // Level 6: Addition and subtraction
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];
        if (cur == "+")
        {
            doSub(From, i, "Add", true);
            break;
        }
        else if (cur == "-")
        {
            doSub(From, i, "Sub", true);
            break;
        }
    }

    // Level 7: Bitwise
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "^")
        {
            doSub(From, i, "Xor", true);
            break;
        }
        else if (cur == "<<")
        {
            doSub(From, i, "Lbs", true);
            break;
        }
        else if (cur == ">>")
        {
            doSub(From, i, "Rbs", true);
            break;
        }
        else if (cur == "&")
        {
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            doSub(From, i, "And", true);
            break;
        }
        else if (cur == "|")
        {
            doSub(From, i, "Or", true);
            break;
        }
    }

    return;
}
