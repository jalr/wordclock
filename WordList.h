#ifndef WORD_LIST_H
#define WORD_LIST_H

#include "Word.h"

class WordList {
  private:
    Word words[10];
    uint8_t length = 0;

  public:
    void add(const Word w);
    void show(IAnimator *animator, uint8_t r, uint8_t g, uint8_t b) const;
    void hide(IAnimator *animator) const;

    WordList diff(WordList &diffWords) const;

    const Word &getWord(uint8_t index) const { return words[index]; }
    uint8_t getLength() const { return length; }
};

#endif  /* !WORD_LIST_H */

