#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "tmux.h"

struct highlightlist  all_highlights = LIST_HEAD_INITIALIZER(all_highlights);

int add_highlight(const char *regstr, int fg, int ignorecase){
  struct highlight *tmp_highlight, *new_highlight;
  int flags=REG_EXTENDED | REG_NEWLINE;

  LIST_FOREACH(tmp_highlight, &all_highlights, lentry) {
    if (strcmp(tmp_highlight->regstr, regstr) == 0 && tmp_highlight->ignorecase == ignorecase) {
      free(tmp_highlight->regstr);
      regfree(&tmp_highlight->reg);
      tmp_highlight->regstr = xmalloc(sizeof(char)*strlen(regstr));
      strcpy(tmp_highlight->regstr, regstr);
      tmp_highlight->fg = fg;
      tmp_highlight->ignorecase = ignorecase;
      if(tmp_highlight->ignorecase){
        flags |= REG_ICASE;
      }
      if (regcomp(&tmp_highlight->reg, tmp_highlight->regstr, flags) != 0) {
        fprintf(stderr, "regex compile failed");
        return 1;
      }
      return 0;
    }
  }
  new_highlight = xmalloc(sizeof(struct highlight));
  new_highlight->regstr = xmalloc(sizeof(char)*strlen(regstr));
  strcpy(new_highlight->regstr, regstr);
  new_highlight->fg = fg;
  new_highlight->ignorecase = ignorecase;
  if (new_highlight->ignorecase) {
    flags |= REG_ICASE;
  }
  if (regcomp(&new_highlight->reg, new_highlight->regstr, flags) != 0) {
    fprintf(stderr, "regex compile failed");
    return 1;
  }

	LIST_INSERT_HEAD(&all_highlights, new_highlight, lentry);
  return 0;
}

struct highlight_search_result find_highlight_target(const u_char *target){
  struct highlight *tmp_highlight;
  struct highlight_search_result res = {.find = 0, .start = 0, .end = 0};
  regmatch_t patternMatch;

  LIST_FOREACH(tmp_highlight, &all_highlights, lentry) { 
    if (regexec(&tmp_highlight->reg, target, 1, &patternMatch, 0) == 0){
     regoff_t tmp_startIndex = patternMatch.rm_so;
     regoff_t tmp_endIndex = patternMatch.rm_eo;
     if (tmp_startIndex == -1 || tmp_endIndex == -1) {
         continue;
     }
     if (res.find == 0 || tmp_startIndex < res.start) {
       res.start = tmp_startIndex;
       res.end = tmp_endIndex;
       res.fg = tmp_highlight->fg;
     }
     res.find = 1;
    }
  }
  return res;
}
