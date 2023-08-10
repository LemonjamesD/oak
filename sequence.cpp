/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "sequence.hpp"

// Global instantiations
multiTemplTable templTable;
map<string, __templStructLookupData> templStructData;

// Activates "dirty" mode, where mem alloc and free are allowed
bool insideMethod = false;

unsigned long long int curLine = 1;
string curFile = "";

// Actually used in dump files, not just for debugging.
void debugPrint(const sequence &What, int spaces, ostream &to)
{
    for (int i = 0; i < spaces; i++)
    {
        to << "| ";
    }

    switch (What.info)
    {
    case code_scope:
        to << "code_scope";
        break;
    case code_line:
        to << "code_line";
        break;
    case atom:
        to << "atom";
        break;
    case keyword:
        to << "keyword";
        break;
    }

    to << " w/ raw " << What.raw << ", type " << toStr(&What.type) << ":\n";
    for (auto s : What.items)
    {
        debugPrint(s, spaces + 1, to);
    }

    return;
}

sequence createSequence(const vector<string> &From)
{
    // Clone to feed into the consumptive version
    // vector<string> temp(From);
    list<string> temp;
    for (auto i : From)
    {
        temp.push_back(i);
    }

    vector<sequence> out;

    // Feed to consumptive version
    while (!temp.empty())
    {
        out.push_back(__createSequence(temp));
    }

    if (out.size() == 0)
    {
        return sequence{};
    }
    else if (out.size() == 1)
    {
        return out[0];
    }
    else
    {
        sequence outSeq;
        outSeq.info = code_scope;
        outSeq.type = nullType;

        for (auto i : out)
        {
            outSeq.items.push_back(i);
        }
        return outSeq;
    }
}

// Internal consumptive version: Erases from vector, so not safe for frontend
sequence __createSequence(list<string> &From)
{
    sequence out;
    out.info = code_line;

    // Misc. Invalid cases (common)
    if (From.empty())
    {
        return out;
    }
    else if (From.front().size() > 11 && From.front().substr(0, 11) == "//__LINE__=")
    {
        // Line update special symbol
        string newLineNum = From.front().substr(11);

        curLine = stoull(newLineNum);

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();
        return __createSequence(From);
    }
    else if (From.front() == ",")
    {
        return out;
    }

    // Erasure macro; Erases types or struct members from existence
    // Technically just moves them to another spot, as
    else if (From.front() == "erase!")
    {
        int count = 0;
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();
        list<string> contents;

        do
        {
            if (From.front() == "(")
            {
                count++;
            }
            else if (From.front() == ")")
            {
                count--;
            }

            if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
            {
                if (From.front() != ",")
                {
                    contents.push_back(From.front());
                }
            }

            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
        } while (!From.empty() && count != 0);

        for (auto s : contents)
        {
            if (!((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\'')))
            {
                throw parse_error("All arguments to 'erase!' must be strings.");
            }

            // Trim quotes
            string symb = s.substr(1, s.size() - 2);

            if (structData.count(symb) != 0)
            {
                structData[symb].erased = true;
            }
            else if (table.count(symb) != 0)
            {
                for (int l = 0; l < table[symb].size(); l++)
                {
                    table[symb][l].erased = true;
                }
            }
            else
            {
                cout << tags::yellow_bold
                     << "Warning! No symbols were erase!-ed via the symbol '" << symb << "'\n"
                     << tags::reset;
            }
        }

        return out;
    }

    // Memory Keywords
    else if (From.front() == "alloc!")
    {
        sm_assert(insideMethod, "Memory cannot be allocated outside of an operator-alias method.");

        int count = 0;
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();
        list<string> contents;

        do
        {
            if (From.front() == "(")
            {
                count++;
            }
            else if (From.front() == ")")
            {
                count--;
            }

            if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
            {
                contents.push_back(From.front());
            }

            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
        } while (!From.empty() && count != 0);

        string num = "1";

        sequence temp = __createSequence(contents);
        string name = toC(temp);

        while (contents.front() == ",")
        {
            contents.pop_front();
        }

        sequence numSeq = __createSequence(contents);
        num = toC(numSeq);

        Type tempType = temp.type;
        Type numType = numSeq.type;

        sm_assert(tempType.info == pointer, "'alloc!' returns a pointer.");
        sm_assert(tempType.next != nullptr, "'alloc!' received a malformed first argument.");
        temp.type = *tempType.next;

        if (numType == nullType)
        {
            num = "1";
            numType.info = atomic;
            numType.name = "u128";
        }

        sm_assert(numType.info == atomic && numType.name == "u128", "'alloc!' takes 'u128', not '" + toStr(&numType) + "'.");

        out = getAllocSequence(temp.type, name, num);

        return out;
    }
    else if (From.front() == "free!")
    {
        sm_assert(insideMethod, "Memory cannot be deleted outside of an operator-alias method.");

        int count = 0;
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();
        list<string> contents;

        do
        {
            if (From.front() == "(")
            {
                count++;
            }
            else if (From.front() == ")")
            {
                count--;
            }

            if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
            {
                contents.push_back(From.front());
            }

            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
        } while (!From.empty() && count != 0);

        sequence temp = __createSequence(contents);
        string name = toC(temp);

        Type tempType = temp.type;
        sm_assert(tempType.info == pointer, "'alloc!' returns a pointer.");
        temp.type = *tempType.next;

        out = getFreeSequence(name, false);

        return out;
    }
    else if (From.front() == "free_arr!")
    {
        sm_assert(insideMethod, "Memory cannot be deleted outside of an operator-alias method.");

        int count = 0;
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();
        list<string> contents;

        do
        {
            if (From.front() == "(")
            {
                count++;
            }
            else if (From.front() == ")")
            {
                count--;
            }

            if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
            {
                contents.push_back(From.front());
            }

            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
        } while (!From.empty() && count != 0);

        sequence temp = __createSequence(contents);
        string name = toC(temp);

        Type tempType = temp.type;
        sm_assert(tempType.info == pointer, "'alloc!' returns a pointer.");
        temp.type = *tempType.next;

        out = getFreeSequence(name, true);

        return out;
    }

    // Misc key-characters
    else if (From.front() == ";")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();
        auto toReturn = __createSequence(From);
        toReturn.type = nullType;

        return toReturn;
    }

    // Keywords
    else if (From.front() == "if" || From.front() == "while")
    {
        // Takes a bool code line and a code scope / code line
        out.info = keyword;
        out.raw = From.front();
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Pops the first full sequence from the remaining front
        // This is for the conditional
        out.items.push_back(__createSequence(From));

        sm_assert(out.items[0].type == Type(atomic, "bool"), out.raw + " statement argument must be boolean. Instead, '" + toStr(&out.items[0].type) + "'");
        sm_assert(!From.empty(), "Missing statement after " + out.raw + "");

        // This is for the code chunk
        out.items.push_back(__createSequence(From));

        return out;
    }
    else if (From.front() == "else")
    {
        // Takes a code scope / code line
        out.info = keyword;
        out.raw = From.front();
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        out.items.push_back(__createSequence(From));

        return out;
    }
    else if (From.front() == "let")
    {
        // Get name
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front(); // let
        sm_assert(!From.empty(), "'let' must be followed by something.");

        string name = From.front();
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Gather templating if there is any
        vector<string> generics;
        if (From.front() == "<")
        {
            while (!From.empty() && From.front() != ">")
            {
                if (From.front() != "<" && From.front() != ">" && From.front() != ",")
                {
                    generics.push_back(From.front());
                }

                From.pop_front();
            }

            if (!From.empty() && From.front() == ">")
            {
                From.pop_front();
            }
        }

        // Get type
        if (From.front() == ":")
        {
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();

            if (!From.empty() && From.front() == "struct")
            {
                // addStruct takes in the full struct definition, from
                // let to end curly bracket. So we must first parse this.

                vector<string> toAdd = {"let", name, ":"};

                int count = 0;
                while (count != 0 || (!From.empty() && From.front() != "}" && From.front() != ";"))
                {
                    if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                    {
                        toAdd.push_back(From.front());
                    }

                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                }

                if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                {
                    toAdd.push_back(From.front());
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();

                if (generics.empty())
                {
                    // Non-templated struct
                    addStruct(toAdd);
                }
                else
                {
                    // Templated struct
                    templStructData[name] = __templStructLookupData{generics, toAdd};
                }

                // This should be left out of toC, as it should only be used
                // in the header file during reconstruction.
            }
            else
            {
                // Non-struct definition; Local var, not function
                // NOTE: let a: i32 = 5; is NO LONGER supported!

                sm_assert(generics.empty(), "Variable declaration must not be templated.");

                // Scrape entire definition for this
                vector<string> toAdd = {
                    "let",
                    name,
                    ":",
                };

                while (!From.empty() && From.front() != ";")
                {
                    toAdd.push_back(From.front());
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();

                    if (!From.empty() && From.front() == "=")
                    {
                        throw sequencing_error("Instantiate / assignment combo is not supported.");
                    }
                }

                // This SHOULD appear during toC.
                out.info = code_line;
                out.raw = "";
                out.type = nullType;
                out.items.clear();

                Type type = toType(toAdd);

                out.items.push_back(sequence{nullType, vector<sequence>(), atom, toStrC(&type, name)});

                // Insert into table
                table[name].push_back(__multiTableSymbol{sequence{type, vector<sequence>(), atom, ""}, type});

                // Call constructor (pointers do not get constructors)
                if (type.info != pointer)
                {
                    // Syntactically necessary
                    out.items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});

                    vector<string> newCall = {"New", "(", "@", name, ")"};
                    int garbage = 0;

                    sequence toAppend;
                    toAppend.info = atom;
                    toAppend.type = resolveFunction(newCall, garbage, toAppend.raw);

                    out.items.push_back(toAppend);

                    // out.items.push_back(sequence{nullType, vector<sequence>(), atom, "; New(&" + name + ")"});
                }

                return out;
            }
        }
        else if (From.front() == "(")
        {
            // Function definition

            if (generics.size() == 0)
            {
                // Arguments
                vector<string> toAdd;
                do
                {
                    toAdd.push_back(From.front());
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                } while (From.front() != "{" && From.front() != ";");

                auto type = toType(toAdd);

                auto oldTable = table;

                auto argsWithType = getArgs(type);
                for (pair<string, Type> p : argsWithType)
                {
                    table[p.first].push_back(__multiTableSymbol{sequence(), p.second});
                }

                bool isMethod = false;
                for (char c : name)
                {
                    if ('A' <= c && c <= 'Z')
                    {
                        isMethod = true;
                        break;
                    }
                }

                bool oldSafeness = insideMethod;
                insideMethod = isMethod;

                // Insert symbol
                table[name].push_back(__multiTableSymbol{sequence{}, type});
                if (From.front() == "{")
                {
                    table[name].back().seq = __createSequence(From);
                }

                insideMethod = oldSafeness;

                restoreSymbolTable(oldTable);
            }
            else
            {
                vector<string> toAdd = {"let", name}, returnType;
                while (From.front() != "->")
                {
                    if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                    {
                        toAdd.push_back(From.front());
                    }

                    From.pop_front();
                }

                toAdd.push_back(From.front());
                From.pop_front();

                while (From.front() != "{" && From.front() != ";")
                {
                    if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                    {
                        toAdd.push_back(From.front());
                        returnType.push_back(From.front());
                    }

                    From.pop_front();
                }

                sm_assert(From.front() == "{", "Generic functions must be defined at declaration.");

                // Scrape contents
                int count = 0;
                do
                {
                    if (From.front() == "{")
                    {
                        count++;
                    }
                    else if (From.front() == "}")
                    {
                        count--;
                    }

                    if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                    {
                        toAdd.push_back(From.front());
                    }

                    From.pop_front();
                } while (count != 0);

                // Insert template
                templTable[name].push_back(__template_info{generics, toAdd, returnType});
            }
        }

        return out;
    }
    else if (From.front() == "{")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Save symbol table for later restoration
        auto oldTable = table;

        out.info = code_scope;

        // Code scope.
        int count = 1;
        list<string> curVec;
        while (true)
        {
            if (From.front() == "{")
            {
                count++;
            }
            else if (From.front() == "}")
            {
                count--;

                if (count == 0)
                {
                    out.items.push_back(__createSequence(curVec));
                    curVec.clear();
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                    break;
                }
            }

            if (count == 1 && (From.front() == ";" || From.front() == "}"))
            {
                if (!curVec.empty())
                {
                    out.items.push_back(__createSequence(curVec));
                    out.items.back().type = nullType;
                    curVec.clear();
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            }
            else
            {
                if (!From.empty())
                {
                    curVec.push_back(From.front());
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                }
                else
                {
                    break;
                }
            }
        }

        // Restore symbol table
        // This copies only newly instantiated functions; No other symbols.
        restoreSymbolTable(oldTable);

        // Check if/else validity
        for (int i = 1; i < out.items.size(); i++)
        {
            if (out.items[i].info == keyword && out.items[i].raw == "else")
            {
                if (out.items[i - 1].items.size() != 0 && out.items[i - 1].items[0].info == keyword && out.items[i - 1].items[0].raw == "if")
                {
                    continue;
                }

                sm_assert(out.items[i - 1].info == keyword && out.items[i - 1].raw == "if",
                          "Else statement must be prefixed by if statement.");
            }
        }

        return out;
    }

    // Non-special case; code line.
    // Function calls and template instantiation may occur within.

    while (From.front().size() > 11 && From.front().substr(0, 11) == "//__LINE__=")
    {
        // Line update special symbol
        string newLineNum = From.front().substr(12);
        curLine = stoull(newLineNum);

        // Pop
        From.pop_front();
    }

    sequence temp;
    temp.info = atom;

    int i = 0;

    vector<string> tempVec;
    for (auto i : From)
    {
        tempVec.push_back(i);
    }
    temp.type = resolveFunction(tempVec, i, temp.raw);

    out.items.push_back(temp);

    // Erase old
    for (int k = 0; !From.empty() && k < i; k++)
    {
        From.pop_front();
    }

    if (out.items.size() == 1)
    {
        return out.items[0];
    }

    return out;
} // __createSequence

// Turn a .oak sequence into a .cpp one
string toC(const sequence &What)
{
    string out = "";

    switch (What.info)
    {
    case code_line:
        for (int i = 0; i < What.items.size(); i++)
        {
            out += toC(What.items[i]);

            if (i + 1 != What.items.size())
            {
                out += " ";
            }
        }

        break;

    case code_scope:
        out += "{\n";

        for (int i = 0; i < What.items.size(); i++)
        {
            if (What.items[i].info == keyword)
            {
                out += toC(What.items[i]);
            }
            else if (What.items[i].type == nullType)
            {
                out += toC(What.items[i]) + ";\n";
            }
            else
            {
                out += "return (" + toC(What.items[i]) + ");\n";
            }
        }

        out += "}\n";

        break;

    case atom:
        out += What.raw;
        break;

    case keyword:
        out += What.raw + " ";

        for (auto child : What.items)
        {
            out += toC(child) + " ";
        }

        break;
    }

    return out;
}

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T)
{
    Type temp(T);

    int count = 0;
    for (Type *cur = &temp; cur != nullptr; cur = cur->next)
    {
        if (cur->info == function)
        {
            count++;
        }

        if (cur->info == maps)
        {
            count--;

            if (count == 0)
            {
                Type out(*cur->next);
                return out;
            }
        }
    }

    return T;
}

vector<pair<string, Type>> getArgs(Type &type)
{
    // Get everything between final function and maps
    // For the case of function pointers, all variable names will be the unit string
    vector<pair<string, Type>> out;
    string curName = "";
    Type curType = nullType;
    Type *cur = &type;
    int count = 0;

    // Dereference
    while (cur != nullptr && cur->info == pointer)
    {
        cur = cur->next;
    }

    // Should now point to first 'function'

    // Do iteration
    while (cur != nullptr)
    {
        if (cur->info == function)
        {
            count++;
        }
        else if (cur->info == maps)
        {
            count--;

            if (count == 0)
            {
                // Final append
                out.push_back(make_pair(curName, curType));
                break;
            }

            // Skip to the end of the return type
            int subCount = 1;
            curType.append(cur->info);
            cur = cur->next;
            while (cur != nullptr)
            {
                if (cur->info == function)
                {
                    subCount++;
                }
                else if (cur->info == maps)
                {
                    subCount--;
                }

                if (subCount == 0)
                {
                    if (cur->info == maps || cur->info == join)
                    {
                        break;
                    }
                }

                curType.append(cur->info, cur->name);

                cur = cur->next;
            }

            // cout << __FILE__ << ":" << __LINE__ << " " << toStr(cur) << '\n';

            continue;
        }

        if (count == 1 && cur->info == var_name)
        {
            curName = cur->name;
        }
        else if (count == 1 && cur->info == join)
        {
            out.push_back(make_pair(curName, curType));
            curName = "";
            curType = nullType;
        }
        else if (count != 1 || cur->info != function)
        {
            // Append to curType
            curType.append(cur->info, cur->name);
        }

        // Increment at end
        cur = cur->next;
    }

    /*
    cout << __FILE__ << ":" << __LINE__ << " output arguments:\n";
    for (auto p : out)
    {
        cout << '\t' << p.first << '\t' << toStr(&p.second) << '\n';
    }
    */

    // Return
    return out;
}

// This should only be called after method replacement
Type resolveFunction(const vector<string> &What, int &start, string &c)
{
    if (What.empty() || start >= What.size())
    {
        return nullType;
    }

    // Pointer check
    if (What[start] == "@")
    {
        start++;

        string followingC = "";
        Type type = resolveFunction(What, start, followingC);
        type.prepend(pointer);

        c += "&" + followingC;

        return type;
    }
    else if (What[start] == "^")
    {
        start++;

        string followingC = "";
        Type type = resolveFunction(What, start, followingC);

        sm_assert(type.info == pointer, "Cannot dereference non-pointer.");
        Type temp = *type.next;
        type = temp;

        c += "*" + followingC;

        return type;
    }

    // Semicolon check
    else if (What[start] == ";")
    {
        c += ";";
        return nullType;
    }

    // Special symbol check
    else if (What[start].size() >= 2 && What[start].substr(0, 2) == "//")
    {
        start++;
        return resolveFunction(What, start, c);
    }

    // Standard case

    // get name (first item)
    string name = What[start];
    Type type = nullType;

    // Parenthesis
    if (What[start] == "(")
    {
        vector<string> toUse;
        int count = 0;
        do
        {
            if (What[start] == "(")
            {
                count++;
            }
            else if (What[start] == ")")
            {
                count--;
            }

            toUse.push_back(What[start]);

            start++;
        } while (start < What.size() && count != 0);

        if (!toUse.empty())
        {
            toUse.erase(toUse.begin());
            toUse.pop_back();
        }

        int trash = 0;
        Type toReturn = resolveFunction(toUse, trash, c);
        c = "(" + c + ")";

        return toReturn;
    }

    // Function call
    else if (What.size() > start + 1 && What[start + 1] == "(")
    {
        // get args within parenthesis
        vector<string> curArg;
        vector<vector<string>> args;

        int count = 0;
        start++;
        do
        {
            if (What[start] == "(")
            {
                count++;
            }
            else if (What[start] == ")")
            {
                count--;
            }

            if (What[start] == "," && count == 1)
            {
                args.push_back(curArg);
                curArg.clear();
            }

            curArg.push_back(What[start]);

            start++;
        } while (start < What.size() && count != 0);

        if (!curArg.empty())
        {
            args.push_back(curArg);
        }

        // Erase commas, open parenthesis on first one
        for (int i = 0; i < args.size(); i++)
        {
            if (!args[i].empty())
            {
                args[i].erase(args[i].begin());
            }
        }

        // Erase end parenthesis
        if (!args.empty() && !args.back().empty())
        {
            args.back().pop_back();
        }

        for (int i = 0; i < args.size(); i++)
        {
            if (args[i].size() == 0)
            {
                args.erase(args.begin() + i);
                i--;
            }
        }

        vector<Type> argTypes;
        vector<string> argStrs;
        for (vector<string> arg : args)
        {
            int trash = 0;
            string cur;

            argTypes.push_back(resolveFunction(arg, trash, cur));
            argStrs.push_back(cur);
        }

        // Special case; Pointer array access
        if (name == "Get" && argTypes.size() == 2)
        {
            if (argTypes[0].info == pointer &&
                argTypes[0].next != nullptr &&
                argTypes[0].next->info == pointer)
            {
                c += argStrs[0] + "[" + argStrs[1] + "]";

                return *argTypes[0].next;
            }
        }

        // Special case; Function pointer copy
        else if (name == "Copy" && argTypes.size() == 2)
        {
            // IE Copy(^^(i32) -> i32, ^(what: i32) -> i32)
            // Double pointer in first, single in second
            // Identical types after pointer stuff

            if (argTypes[0].info == pointer &&
                argTypes[0].next != nullptr &&
                argTypes[0].next->info == pointer &&
                argTypes[0].next->next != nullptr &&
                argTypes[0].next->next->info == function &&
                argTypes[1].info == pointer &&
                argTypes[1].next != nullptr &&
                argTypes[1].next->info == function)
            {
                // Ensure equality; If not, throw error
                Type *left, *right;

                left = argTypes[0].next->next;
                right = argTypes[1].next;

                bool isValid = true;
                while (left != nullptr && right != nullptr)
                {
                    // DO ITERATION HERE
                    while (left->info == var_name)
                    {
                        left = left->next;
                    }
                    while (right->info == var_name)
                    {
                        right = right->next;
                    }

                    if (left->info != right->info || left->name != right->name)
                    {
                        isValid = false;
                        break;
                    }

                    left = left->next;
                    right = right->next;
                }

                sm_assert(isValid, "Cannot assign fn ptr '" + toStr(&argTypes[1]) + "' to '" + toStr(&argTypes[0]) + "'.");

                // Turn to C
                c += "(*" + argStrs[0] + ") = " + argStrs[1];

                // Return nullType (Copy yields void)
                return nullType;
            }
        }

        c += name + "(";
        for (int j = 0; j < argStrs.size(); j++)
        {
            if (j != 0)
            {
                c += ", ";
            }

            c += argStrs[j];
        }
        c += ")";

        // Search for candidates
        sm_assert(table.count(name) != 0, "Function call '" + name + "' has no registered symbols.");
        sm_assert(table[name].size() != 0, "Function call '" + name + "' has no registered symbols.");

        auto candidates = table[name];

        // Weed out any candidates which do not have the correct argument types
        for (int j = 0; j < candidates.size(); j++)
        {
            Type curType = candidates[j].type;
            while (curType != nullType && curType.info == pointer)
            {
                popTypeFront(curType);
            }

            auto candArgs = getArgs(curType);
            for (int k = 0; k < candArgs.size(); k++)
            {
                if (candArgs[k].second == nullType)
                {
                    candArgs.erase(candArgs.begin() + k);
                    k--;
                }
            }

            if (candArgs.size() != argTypes.size() || candidates[j].erased)
            {
                candidates.erase(candidates.begin() + j);
                j--;
                continue;
            }

            for (int k = 0; k < candArgs.size(); k++)
            {
                if (!typesAreSame(&candArgs[k].second, &argTypes[k]))
                {
                    candidates.erase(candidates.begin() + j);
                    j--;
                    break;
                }
            }
        }

        // If no candidates, throw error
        if (candidates.size() == 0)
        {
            // Get all theoretical candidates again
            candidates = table[name];

            cout << "\nCandidates:\n";
            for (auto c : candidates)
            {
                cout << name << " has type " << toStr(&c.type) << '\n'
                     << "Candidate arguments (";
                auto candArgs = getArgs(c.type);
                cout << candArgs.size() << "):\n";

                for (auto arg : candArgs)
                {
                    cout << "\t" << arg.first << "\t" << toStr(&arg.second) << '\n';
                }

                // Provide reason for elimination
                if (candArgs.size() != argTypes.size())
                {
                    cout << "\t(Too " << ((candArgs.size() < argTypes.size()) ? "few" : "many")
                         << " arguments)\n";
                }
                else if (c.erased)
                {
                    cout << "\t(Erased / private symbol)\n";
                }
                else
                {
                    cout << "\t(Incompatible argument type(s))\n";
                }
            }

            cout << "\nProvided arguments (" << argTypes.size() << "):\n(";
            for (auto a : argTypes)
            {
                cout << toStr(&a) << ", ";
            }
            cout << ")\n\n";

            throw sequencing_error("Function call '" + name + "' has no viable candidates.");
        }

        // If multiple candidates, ensure all return types are the same
        else if (candidates.size() > 1)
        {
            type = getReturnType(candidates[0].type);

            for (int j = 1; j < candidates.size(); j++)
            {
                if (getReturnType(candidates[j].type) != type)
                {
                    throw sequencing_error("Function call '" + name + "' has two or more viable candidates.");
                }
            }

            // Now safe; Has been verified that all candidates have identical types
        }

        // Single candidate
        type = getReturnType(candidates[0].type);

        start--;
    }

    // Function template instantiation (not struct at this stage)
    // TODO: Modernize for multi-symbol generic replacements
    else if (What.size() > start + 1 && What[start + 1] == "<")
    {
        vector<string> generics;
        start += 2;

        while (start < What.size() && What[start] != ">")
        {
            if (What[start] != "<" && What[start] != ">" && What[start] != ",")
            {
                generics.push_back(What[start]);
            }

            start++;
        }
        start++;

        sm_assert(start >= What.size() || What[start] == ";", "Template instantiation call must end with semicolon, not '" + What[start] + "'");

        try
        {
            // Check for function formats
            instantiateTemplate(name, generics);
        }
        catch (runtime_error &e)
        {
            if (templStructData.count(name) == 0)
            {
                throw e;
            }
            else
            {
                instantiateStruct(name, generics);
            }
        }
    }

    else
    {
        // Non-function-call

        // Literal check
        Type litType = checkLiteral(What[start]);
        if (litType == nullType)
        {
            // Is not a literal
            sm_assert(table.count(What[start]) != 0, "No definitions exist for symbol '" + What[start] + "'.");
            auto candidates = table[What[start]];
            sm_assert(candidates.size() != 0, "No definitions exist for symbol '" + What[start] + "'.");

            type = candidates.back().type;
        }
        else
        {
            // Is a literal
            type = litType;
        }

        c += What[start];
    }

    // if member access, resolve that
    while (start < What.size() && What[start + 1] == ".")
    {
        // Auto-dereference pointers (. to -> automatically)
        if (type.info == pointer)
        {
            while (type.info == pointer)
            {
                Type temp = *type.next;
                type = temp;

                c = "*" + c;
            }

            c = "(" + c + ")";
        }

        sm_assert(type.info == atomic, "Error during type trimming for member access; Could not find struct name.");
        string structName = type.name;

        sm_assert(structData.count(structName) != 0, "Struct type '" + structName + "' does not exist.");
        sm_assert(!structData[structName].erased, "Struct '" + structName + "' exists, but is erased (private).");
        sm_assert(structData[structName].members.count(What[start + 2]) != 0, "Struct '" + structName + "' has no member '" + What[start + 2] + "'.");

        type = structData[structName].members[What[start + 2]];

        c += "." + What[start + 2];

        start += 2;
    }

    start++;

    // Return function return type
    return type;
}

// Ignores all var_names
bool typesAreSame(Type *A, Type *B)
{
    Type *left, *right;
    left = A, right = B;

    while (left != nullptr && right != nullptr)
    {
        while (left != nullptr && left->info == var_name)
        {
            left = left->next;
        }

        while (right != nullptr && right->info == var_name)
        {
            right = right->next;
        }

        if (left->info != right->info)
        {
            // Failure
            return false;
        }
        else
        {
            if (left->info == atomic && left->name != right->name)
            {
                // Failure
                return false;
            }
        }

        // Success; Move on
        left = left->next;
        right = right->next;
    }

    return true;
}

Type checkLiteral(const string &From)
{
    // Check as bool
    if (From == "true" || From == "false")
    {
        return Type(atomic, "bool");
    }

    // Check as string
    else if ((From.front() == '"' && From.back() == '"') || (From.front() == '\'' && From.back() == '\''))
    {
        return Type(atomic, "str");
    }

    // Check as numbers
    bool canBeDouble = true;
    for (char c : From)
    {
        if (!('0' <= c && c <= '9') && c != '-' && c != '.')
        {
            canBeDouble = false;
            break;
        }
    }

    if (canBeDouble)
    {
        if (From.find(".") == string::npos)
        {
            // Int

            // Check size
            long long val = stoll(From);

            if (abs(val) <= __INT_MAX__)
            {
                return Type(atomic, "i32");
            }
            else if (abs(val) <= __LONG_MAX__)
            {
                return Type(atomic, "i64");
            }
            else
            {
                return Type(atomic, "i128");
            }
        }
        else
        {
            // Float
            return Type(atomic, "f64");
        }
    }

    // Default: Not a literal
    return nullType;
}

void restoreSymbolTable(multiSymbolTable &backup)
{
    multiSymbolTable newTable = backup;
    for (auto p : table)
    {
        if (p.second.size() == backup[p.first].size())
        {
            continue;
        }
        else
        {
            for (auto s : p.second)
            {
                if (s.type.info == function)
                {
                    bool present = false;
                    for (auto ss : backup[p.first])
                    {
                        if (ss.type == s.type)
                        {
                            present = true;
                            break;
                        }
                    }

                    if (!present)
                    {
                        newTable[p.first].push_back(s);
                    }
                }
            }
        }
    }

    table = newTable;

    return;
}

string getStructCanonicalName(const string &Name, const vector<string> &GenericReplacements)
{
    string out = Name;
    out += "__";
    for (int i = 0; i < GenericReplacements.size(); i++)
    {
        out += GenericReplacements[i] + "_";
    }

    return out;
}

__structLookupData *instantiateStruct(const string &Name, const vector<string> &GenericReplacements)
{
    // Examine and verify candidates (uses raw name)
    if (templStructData.count(Name) == 0)
    {
        throw parse_error("No candidates for templated struct '" + Name + "'");
    }
    auto candidate = templStructData[Name];
    if (candidate.generics.size() != GenericReplacements.size())
    {
        throw parse_error("Cannot instantiate template with generic count " +
                          to_string(candidate.generics.size()) + " with only " +
                          to_string(GenericReplacements.size()) + " generics.");
    }

    // Create canonical name
    string realName = getStructCanonicalName(Name, GenericReplacements);

    // Do substitutions
    for (int genericInd = 0; genericInd < candidate.generics.size(); genericInd++)
    {
        for (int i = 0; i < candidate.guts.size(); i++)
        {
            if (candidate.guts[i] == candidate.generics[genericInd])
            {
                candidate.guts[i] = GenericReplacements[genericInd];
            }
        }
    }

    candidate.guts[1] = realName;

    // Add struct
    addStruct(candidate.guts);

    // Return newly instantiated struct type
    return &structData[realName];
}

__multiTableSymbol *instantiateTemplate(const string &Name, const vector<string> &GenericReplacements)
{
    // Examine candidates
    auto candidates = templTable[Name];
    if (candidates.size() == 0)
    {
        throw parse_error("No candidates exist for templated function '" + Name + "'");
    }
    bool found = false;
    __template_info toUse;
    for (int i = 0; i < candidates.size(); i++)
    {
        if (candidates[i].generics.size() == GenericReplacements.size())
        {
            found = true;
            toUse = candidates[i];
            break;
        }
    }

    if (!found)
    {
        for (auto c : candidates)
        {
            cout << "Candidate '" << Name << "' had " << c.generics.size() << " generics:\n\t";
            for (auto g : c.generics)
            {
                cout << '\t' << g;
            }
            cout << '\n';
        }

        cout << "Required generic count: " << GenericReplacements.size() << '\n';

        throw parse_error("No template candidates match generics count for function '" + Name + "'");
    }

    // At this point we can be assured that our template is legit

    // Do replacing
    for (int i = 0; i < GenericReplacements.size(); i++)
    {
        // Replace in guts
        for (int j = 0; j < toUse.guts.size(); j++)
        {
            if (toUse.guts[j] == toUse.generics[i])
            {
                toUse.guts[j] = GenericReplacements[i];
            }
        }

        // Replace in return type
        for (int j = 0; j < toUse.returnType.size(); j++)
        {
            if (toUse.returnType[j] == toUse.generics[i])
            {
                toUse.returnType[j] = GenericReplacements[i];
            }
        }
    }

    Type outType = toType(toUse.returnType);

    // Scan for existing instantiation and, if it exists, return it
    auto instantCandidates = table[Name];
    for (int i = 0; i < instantCandidates.size(); i++)
    {
        if (instantCandidates[i].type == outType)
        {
            cout << tags::yellow_bold
                 << "Warning! Generic function "
                 << Name
                 << toStr(&outType)
                 << " has already been instantiated;\n"
                 << "Repeated instantiation calls are redundant.\n"
                 << tags::reset;

            return &table[Name][i];
        }
    }

    // Recursion-safe way
    table[Name].push_back(__multiTableSymbol{createSequence(toUse.guts), outType});

    return &table[Name].back();
}
