/* LXX, a project dedicated to find citations of the
 * Septuagint in the Greek New Testament.
 * This code is in the public domain
 * and contributed by Zoltán Kovács <zoltan@geogebra.org>.
 */

#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

#include "swmgr.h"
#include "swmodule.h"
#include "swtext.h"
#include "markupfiltmgr.h"
#include "utf8greekaccents.h"

using namespace std;
using namespace sword;

// The number of matching words must be at least...
#define MIN_CHARS_MATCH 20

// This is hardcoded, it contains the index of the last verse
// in the New Testament in Sword.
#define MAX_VERSES 32360

string keys[MAX_VERSES];
string verses[MAX_VERSES];

long sStart;
long sEnd;
long lStart;
long lEnd;

string processWord(string word) {
    string rewritten;
    for (int i = 0; i < word.length(); i += 2) {
        auto c1 = static_cast<unsigned char>(word[i]);
        auto c2 = static_cast<unsigned char>(word[i + 1]);
        // keep care of special characters
        if (c1 == 0x80) {
            i--;
        }
        // keep only Greek characters
        if (c1 == 0xCE || c1 == 0xCF) {
            // force lowercase
            if (c1 == 0xCE && c2 < 0xB1 &&
                !(c2 >= 0xA0 && c2 <= 0xA9)) {
                // letters before Pi (excluding Chi, Pi, Omega, Y, Sigma, Tau, Psi, Phi)
                c2 += 0x20;
            }
            if (c1 == 0xCE && c2 >= 0xA0 && c2 <= 0xA9) {
                // Chi, Pi, Omega, Y, Sigma, Tau, Psi, Phi
                c1 = 0xCF;
                c2 -= 0x20;
            }
            if (c1 == 0xCF && c2 < 0x80) {
                c2 += 0x20;
            }
            rewritten.push_back(c1);
            rewritten.push_back(c2);
        }
    }
    return rewritten;
}

/* Remove double spaces. */
string printVerse(const string &verse) {
    string out;
    string word;
    stringstream ss(verse);

    while (ss >> word) {
        out.append(word).append(" ");
    }

    // remove last space
    out.pop_back();
    return out;
}

/* Add double spaces. */
string processVerse(const string &verse) {
    string out;
    string word;
    stringstream ss(verse);

    while (ss >> word) {
        word = processWord(word);
        out.append(word);
        // use double space to ease comparison later
        out.append("  ");
    }

    return out;
}

void loadVerses(string ls, string Ls, string ss, string Ss) {
    SWMgr library(new MarkupFilterMgr(FMT_PLAIN));
    SWModule *lxx, *sblgnt;
    lxx = library.getModule("LXX");
    if (!lxx) {
        cerr << "The SWORD module LXX is not installed" << "\n";
        exit(1);
    }
    sblgnt = library.getModule("SBLGNT");
    if (!sblgnt) {
        cerr << "The SWORD module SBLGNT is not installed" << "\n";
        exit(1);
    }
    if (ls.length() == 0) {
        ls = "Genesis 1:1";
    }
    if (Ls.length() == 0) {
        Ls = "Malachi 4:6";
    }
    if (ss.length() == 0) {
        ss = "Matthew 1:1";
    }
    if (Ss.length() == 0) {
        Ss = "Revelation 22:21";
    }
    lxx->setKey(ls.c_str());
    lStart = lxx->getIndex(); // 4
    lxx->setKey(Ls.c_str());
    lEnd = lxx->getIndex(); // 24114
    sblgnt->setKey(ss.c_str());
    sStart = sblgnt->getIndex(); // 24118
    sblgnt->setKey(Ss.c_str());
    sEnd = sblgnt->getIndex(); // 32359

    SWBuf lVerse, sVerse;
    SWOptionFilter *filter = new UTF8GreekAccents();
    filter->setOptionValue("off");

    cerr << "Loading LXX..." << flush;
    // Iterate on verses of LXX:
    for (long l = lStart; l <= lEnd; ++l) {
        lxx->setIndex(l);
        lVerse = lxx->renderText();
        // Some entries may be empty (e.g. book or chapter titles):
        if (lVerse.size() > 0) {
            keys[l] = lxx->getKeyText();
            verses[l] = processVerse(lVerse.c_str());
        }
    }
    cerr << " done" << "\n";

    cerr << "Loading SBLGNT (without accents)..." << flush;
    // Iterate on verses of NT:
    for (long s = sStart; s <= sEnd; ++s) {
        // Lookup NT text:
        sblgnt->setIndex(s);
        sVerse = sblgnt->renderText();
        if (sVerse.size() > 0) {
            // Remove accents from the NT text:
            filter->processText(sVerse);
            keys[s] = sblgnt->getKeyText();
            verses[s] = processVerse(sVerse.c_str());
        }
    }
    cerr << " done" << "\n";
}

int compareVerses(long v1i, long v2i) {
    string v1 = verses[v1i];
    string v2 = verses[v2i];
    int v1l = v1.length() / 2;

    for (int w = v1l; w >= MIN_CHARS_MATCH; --w) {
        for (int s = 0; s <= v1l - w; ++s) {
            string sub = v1.substr(static_cast<unsigned long>(s * 2),
                                   static_cast<unsigned long>(w * 2));
            int start_ok = false;
            if (s == 0) {
                start_ok = true;
            } else {
                string start = v1.substr(s * 2 - 2, 2);
                if (start == "  ") {
                    start_ok = true;
                }
            }
            if (start_ok) {
                string end = sub.substr(sub.length() - 2, 2);
                if (end == "  " || (s + w) * 2 == v1.length()) {
                    // Do comparison only for whole words
                    int found = v2.find(sub);
                    if (found != string::npos) {
                        sub = sub.substr(0, sub.length() - 2);
                        string out;
                        out = keys[v1i];
                        out.append(": ");
                        if (s > 0) {
                            out.append(v1.substr(0, s * 2));
                        }
                        out.append("**").append(sub).append("**");
                        if ((s + w) * 2 < v1.length()) {
                            out.append("  ").append(v1.substr((s + w) * 2));
                        }

                        out.append(" → ").append(keys[v2i]).append(": ");
                        if (found > 0) {
                            out.append(v2.substr(0, found));
                        }
                        out.append("**").append(sub).append("**");
                        if (found + w * 2 < v2.length()) {
                            out.append("  ").append(v2.substr(found + w * 2));
                        }
                        out.append("\n");
                        cout << printVerse(out) << "\n" << flush;

                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void compareVerses(long l1, long lN, long s1, long sN) {
    for (long l = l1; l <= lN; ++l) {
        if (verses[l].length() > 0) {
            for (long s = s1; s <= sN; ++s) {
                compareVerses(l, s);
            }
        }
    }
}

void compareVersesSwapLoops(long l1, long lN, long s1, long sN) {
    for (long s = s1; s <= sN; ++s) {
        if (verses[s].length() > 0) {
            for (long l = l1; l <= lN; ++l) {
                compareVerses(l, s);
            }
        }
    }
}

// @author Iain Hull (https://stackoverflow.com/a/868894/1044586)
class InputParser {
public:
    InputParser(int &argc, char **argv) {
        for (int i = 1; i < argc; ++i)
            this->tokens.emplace_back(argv[i]);
    }

    const string &getCmdOption(const string &option) const {
        vector<string>::const_iterator itr;
        itr = find(this->tokens.cbegin(), this->tokens.cend(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
            return *itr;
        }
        static const string empty_string;
        return empty_string;
    }

    bool cmdOptionExists(const string &option) const {
        return find(this->tokens.begin(), this->tokens.end(), option)
               != this->tokens.end();
    }

private:
    vector<string> tokens;
};

void showHelp(const string &executable) {
    cout << "Usage: " << executable << " [options]\n";
    cout << " where options can be:\n";
    cout << " -h            this help\n";
    cout << " -l <verse>    first LXX verse to lookup (e.g. \"Genesis 1:1\")\n";
    cout << " -L <verse>    last LXX verse to lookup\n";
    cout << " -s <verse>    first SBLGNT verse to lookup\n";
    cout << " -S <verse>    last SBLGNT verse to lookup\n";
    cout << " -n            sort the output by New Testament matches\n";
}

int main(int argc, char **argv) {
    InputParser input(argc, argv);
    if (input.cmdOptionExists("-h")) {
        showHelp(argv[0]);
        exit(0);
    }
    string l, L, s, S;
    if (input.cmdOptionExists("-l")) {
        l = input.getCmdOption("-l");
    }
    if (input.cmdOptionExists("-L")) {
        L = input.getCmdOption("-L");
    }
    if (input.cmdOptionExists("-s")) {
        s = input.getCmdOption("-s");
    }
    if (input.cmdOptionExists("-S")) {
        S = input.getCmdOption("-S");
    }
    loadVerses(l, L, s, S);
    // compareVerses(30,25605);
    if (input.cmdOptionExists("-n")) {
        compareVersesSwapLoops(lStart, lEnd, sStart, sEnd);
    } else {
        compareVerses(lStart, lEnd, sStart, sEnd);
    }
    return 0;
}
