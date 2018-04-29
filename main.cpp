/* LXX, a project dedicated to find citations of the
 * Septuaginta in the Greek New Testament.
 * This code is in the public domain
 * and contributed by Zoltán Kovács <zoltan@geogebra.org>.
 */

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

// Keep only words with a minimum length of...
#define MIN_WORD_LENGTH 5
// The number of matching words must be at least...
#define MIN_WORDS_MATCH 3

// This is hardcoded, it contains the index of the last verse
// in the New Testament in Sword.
#define MAX_VERSES 32360
// Greek UTF8 characters are stored on two bytes
#define _MIN_WORD_LENGTH (MIN_WORD_LENGTH * 2)

string keys[MAX_VERSES];
string verses[MAX_VERSES];

long sstart;
long send;
long lstart;
long lend;

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

string processVerse(const string &verse) {
    string word;
    string truncatedVerse;
    stringstream ss(verse);

    while (ss >> word) {
        word = processWord(word);
        if (word.length() >= _MIN_WORD_LENGTH) {
            word.resize(_MIN_WORD_LENGTH);
            truncatedVerse.append(word);
            truncatedVerse.append(" ");
        }
    }

    return truncatedVerse;
}

void loadVerses() {
    SWMgr library(new MarkupFilterMgr(FMT_PLAIN));
    SWModule *lxx, *sblgnt;
    lxx = library.getModule("LXX");
    sblgnt = library.getModule("SBLGNT");
    lxx->setKey("Genesis 1:1");
    lstart = lxx->getIndex(); // 4
    lxx->setKey("Malachi 4:6");
    lend = lxx->getIndex(); // 24114
    sblgnt->setKey("Matthew 1:1");
    sstart = sblgnt->getIndex(); // 24118
    sblgnt->setKey("Revelation 22:21");
    send = sblgnt->getIndex(); // 32359

    SWBuf lverse, sverse;
    SWOptionFilter *filter = new UTF8GreekAccents();
    filter->setOptionValue("off");

    cout << "Loading LXX...";
    // Iterate on verses of LXX:
    for (long l = lstart; l <= lend; ++l) {
        lxx->setIndex(l);
        lverse = lxx->renderText();
        // Some entries may be empty (e.g. book or chapter titles):
        if (lverse.size() > 0) {
            keys[l] = lxx->getKeyText();
            verses[l] = processVerse(lverse.c_str());
        }
    }
    cout << " done" << "\n";

    cout << "Loading SBLGNT (without accents)...";
    // Iterate on verses of NT:
    for (long s = sstart; s <= send; ++s) {
        // Lookup NT text:
        sblgnt->setIndex(s);
        sverse = sblgnt->renderText();
        if (sverse.size() > 0) {
            // Remove accents from the NT text:
            filter->processText(sverse);
            keys[s] = sblgnt->getKeyText();
            verses[s] = processVerse(sverse.c_str());
        }
    }
    cout << " done" << "\n";
}

int compareVerses(long v1i, long v2i) {
    string v1 = verses[v1i];
    string v2 = verses[v2i];
    int v1l = v1.length();
    int v1w = v1l / (_MIN_WORD_LENGTH + 1);
    // cout << "v1=" << v1 << "(" << v1w << ")\n";
    // cout << "v2=" << v2 << "(" << v2w << ")\n";

    for (int w = v1w; w >= MIN_WORDS_MATCH; --w) {
        for (int s = 0; s <= v1w - w; ++s) {
            string sub = v1.substr(static_cast<unsigned long>(s * (_MIN_WORD_LENGTH + 1)),
                                   static_cast<unsigned long>(w * (_MIN_WORD_LENGTH + 1)));
            // cout << "Checking " << sub << "\n";
            int found = v2.find(sub);
            if (found != string::npos) {
                // cout << "Found at position " << found << " \n";
                return true;
            }
        }
    }
    return false;
}

void compareVerses(long l1, long lN, long s1, long sN) {
    for (long l = l1; l < lN; ++l) {
        if (verses[l].length() > 0) {
            for (long s = s1; s < sN; ++s) {
                if (compareVerses(l, s)) {
                    cout << keys[l] << " (" << l
                         << ") seems to be cited in " << keys[s]
                         << " (" << s << ")\n";
                }
            }
        }
    }
}

void fullCompare() {
    compareVerses(lstart, lend, sstart, send);
}

int main() {
    loadVerses();
    // compareVerses(30,25605);
    fullCompare();
    return 0;
}

