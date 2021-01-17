#ifndef _PATH_H
#define _PATH_H

#define isdirsep(ch) ( { char __c = (ch); ((__c) == '/' || (__c) == '\\' ); } )

#define issep(ch) (strchr (" \t\n\r", (ch)) != NULL)

#define isabspath(p) \
  (isdirsep (*(p)) || (isalpha (*(p)) && (p)[1] == ':' && (!(p)[2] || isdirsep ((p)[2]))))

#endif
