#ifndef RECCHECK
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <random>
#include <iomanip>
#include <fstream>
#include <exception>
#endif

#include "boggle.h"

std::vector<std::vector<char> > genBoard(unsigned int n, int seed)
{
    std::mt19937 r(seed);

    int freq[26] = {9,2,2,4,12,2,3,2,9,1,1,4,2,6,8,2,1,6,4,6,4,2,2,1,2,1};
    std::vector<char> letters;
    for(char c='A'; c<='Z';c++)
    {
        for(int i=0;i<freq[c-'A'];i++)
        {
            letters.push_back(c);
        }
    }
    std::vector<std::vector<char> > board(n);
    for(unsigned int i=0;i<n;i++)
    {
        board[i].resize(n);
        for(unsigned int j=0;j<n;j++)
        {
            board[i][j] = letters[(r() % letters.size())];
        }
    }
    return board;
}

void printBoard(const std::vector<std::vector<char> >& board)
{
    unsigned int n = board.size();
    for(unsigned int i=0;i<n;i++)
    {
        for(unsigned int j=0;j<n;j++)
        {
            std::cout << std::setw(2) << board[i][j];
        }
        std::cout << std::endl;
    }
}

std::pair<std::set<std::string>, std::set<std::string> > parseDict(std::string fname)
{
    std::ifstream dictfs(fname.c_str());
    if(dictfs.fail())
    {
        throw std::invalid_argument("unable to open dictionary file");
    }
    std::set<std::string> dict;
    std::set<std::string> prefix;
    std::string word;
    while(dictfs >> word)
    {
        dict.insert(word);
        for(unsigned int i=word.size()-1;i>=1;i--)
        {
            prefix.insert(word.substr(0,i));
        }
    }
    prefix.insert("");
    return make_pair(dict, prefix);
}

std::set<std::string> boggle(
    const std::set<std::string>& dict,
    const std::set<std::string>& prefix,
    const std::vector<std::vector<char> >& board)
{
    std::set<std::string> result;
    for(unsigned int i=0;i<board.size();i++)
    {
        for(unsigned int j=0;j<board.size();j++)
        {
            boggleHelper(dict, prefix, board, "", result, i, j, 0, 1);
            boggleHelper(dict, prefix, board, "", result, i, j, 1, 0);
            boggleHelper(dict, prefix, board, "", result, i, j, 1, 1);
        }
    }

    return result;
}

bool boggleHelper(
    const std::set<std::string>& dict,
    const std::set<std::string>& prefix,
    const std::vector<std::vector<char> >& board,
    std::string word,
    std::set<std::string>& result,
    unsigned int r,
    unsigned int c,
    int dr,
    int dc)
{
    if(r >= board.size() || c >= board.size())
    {
        return false;
    }

    word += board[r][c];

    const bool isWord = dict.find(word) != dict.end();
    const bool canContinue = prefix.find(word) != prefix.end();

    bool foundLongerWord = false;
    if(canContinue)
    {
        const int nextR = static_cast<int>(r) + dr;
        const int nextC = static_cast<int>(c) + dc;

        if(nextR >= 0 && nextC >= 0 &&
           nextR < static_cast<int>(board.size()) &&
           nextC < static_cast<int>(board.size()))
        {
            foundLongerWord = boggleHelper(
                dict, prefix, board, word, result,
                static_cast<unsigned int>(nextR),
                static_cast<unsigned int>(nextC),
                dr, dc);
        }
    }

    // Add this word only when no longer dictionary word was found
    // farther along this same line.
    if(!foundLongerWord && isWord)
    {
        result.insert(word);
        return true;
    }

    return foundLongerWord;
}
