# LXX

This project is dedicated to find citations of the
[Septuagint](https://en.wikipedia.org/wiki/Septuagint) in the
[Greek New Testament](https://en.wikipedia.org/wiki/New_Testament).

## Background

The [Bible](https://en.wikipedia.org/wiki/Bible),
a book that has been translated into [many languages](https://en.wikipedia.org/wiki/Bible_translations),
is maybe the most important book has ever existed. Believers
have evidence that it is God's word, written by
various people being inspired by God himself.

The Bible has many internal references. Many of them
connect the two parts of the Bible: the Old Testament
and the New Testament. The books of the Old Testament are
usually written in Hebrew, while the books of the New Testament
are Greek. A natural question can arise: How can Hebrew
texts be cited in the Greek text?

There are several research projects on this,
and today it seems to be clear that many authors of
the New Testament cite a Greek translation of
the Hebrew text, namely the Septuagint, that
was widely popular in the first centuries, and still popular
in some Christian churches. Many
Bible translations, however, mostly use only the
Hebrew text as a basis of the Old Testament,
because the importance of the Septuagint is
not yet widely identified.

## This project

This software project attempts to support finding citations
of the Septuagint in the New Testament automatically.
The work is based on The SWORD Project.

## The method used

The provided software tool compares all verses
of the Septuagint with all verses of the Greek New Testament.
The texts are provided by the SWORD project
from the modules LXX and SBLGNT.

Before comparing two verses, two steps are required:

1. All accents are removed. (This step is performed
only on the text of SBLGNT.)
2. All letters are converted into lowercase.

After these steps an exact match is checked
between two verses, using substrings of whole words,
leaving them in the same consecutive order.

### Output

All matches are printed on the standard output (on the terminal), line by line, like this first match:

Genesis 1:1: εν αρχη εποιησεν ο θεος **τον ουρανον και την γην** → Luke 16:17: ευκοπωτερον δε εστιν **τον ουρανον και την γην** παρελθειν η του νομου μιαν κεραιαν πεσειν 

## Using the software tool

At the moment the project was tested on
Ubuntu Linux 17.10 but it should work
on many other systems.

### Installation

The following commands
should be entered on command line:
```commandline
sudo apt install libsword-common libsword-dev git cmake build-essential
git clone https://github.com/kovzol/lxx.git
cd lxx
cmake .
make
```
Then the modules LXX and SBLGNT need to be downloaded from
[The SWORD Project](https://www.crosswire.org/sword/modules/ModDisp.jsp?modType=Bibles).
Finally the steps at the [For Linux](https://www.crosswire.org/sword/docs/moduleinstall.jsp)
section on the website of The SWORD Project should be followed.

### Running

Enter the command
```commandline
./lxx
```

There are some command line options available. Please use the switch **-h** to learn more on this. 

### Full output

The full output of a simple run of a previous version of the tool
can be found in the file [output.txt](output.txt).
It was produced in about 10 minutes by using a modern personal computer,
but there were some issues with the used method, hence this output
is not accurate and should be handled carefully.

The current version runs much slower, but it should give all matches properly.
This is, however, not fully checked yet. The output for verses in the book of Genesis
can be found in the file [Genesis.txt](Genesis.txt). It took more than an hour
to create this output. 
