#ifndef UTIL_CONF_H
#define UTIL_CONF_H

#include <util_io.h>

#define ITEM_HEADER "[ITEM]"

/* -- LOOKUP TABLES -- */

static char *id_lookup_table[] = {
  "ID_NONE",
  "ID_LESSER_HEALTH_POTION",
  "ID_IRON_SWORD"
};

static char *type_lookup_table[] = {
  "TYPE_NONE",
  "TYPE_CONSUME",
  "TYPE_EQUIP"
};

typedef enum
{
  conf_type_int,
  conf_type_string
} conf_type_e;

typedef struct
{
  char key[32];
  conf_type_e type;
  union
  {
    int i;
    char s[256];
  } conf_var_u;
} conf_var_t;

typedef struct
{
  conf_var_t *vars;
  int key_value_pair_count;
  int success;
} conf_t;

// [checks if token can be found from the lookup table]
// 
// [token] [string token to check]
// 
// [returns the number of the token if found]
// [returns -1 if token was not found]
static int id_lookup(char *token)
{
  for(int i = 0; i < ID_LAST; i++)
  {
    if(!strcmp(token, id_lookup_table[i]))
    {
      return i;
    }
  }

  return -1;
}

// [checks if token can be found from the lookup table]
// 
// [token] [string token to check]
// 
// [returns the number of the token if found]
// [returns -1 if token was not found]
static int type_lookup(char *token)
{
  for(int i = 0; i < TYPE_LAST; i++)
  {
    if(!strcmp(token, type_lookup_table[i]))
    {
      return i;
    }
  }

  return -1;
}

// [checks if the given character is a standard white-space characters]
// 
// [ch] [character to check]
// 
// [returns 1 for true]
// [returns 0 for false]
static int is_space(int ch)
{
  if(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r')
  {
    return 1;
  }

  return 0;
}

// [checks if the given string is a number or not]
// 
// [str] [string to check]
// 
// [returns 1 for true]
// [returns 0 for false]
static int is_number(char *str)
{
  // handle cases where the pointer is NULL
  // the character pointed to is a null-terminator
  // or one of the standard white-space characters
  if(str == NULL || *str == '\0' || is_space(*str))
  {
    return 0;
  }

  char *p;
  strtod(str, &p);

  // if the value was a number, *p will equal to '\0' and the return value is 1
  // if the value was a character, *p will equal to the first letter and the return value is 0
  return *p == '\0';
}

// [get the conf_var_t of a key]
// 
// [conf] [conf_t pointer]
// [key] [conf key value]
// 
// [returns a pointer to a conf_var_t structure]
static conf_var_t* conf_get_var(conf_t *conf, char *key)
{
  if(!conf->success)
  {
    return NULL;
  }

  for(int i = 0; i < conf->key_value_pair_count; i++)
  {
    if(!strcmp(conf->vars[i].key, key))
    {
      return &conf->vars[i];
    }
  }

  return NULL;
}

// [load conf file into a conf_t struct]
// 
// [conf] [conf_t pointer]
// [path] [path to the file]
// 
// [returns 1 for success]
// [returns 0 for failure]
static int conf_load(conf_t *conf, char *path)
{
  printf("Loading config file %s\n", path);

  conf->success = 0;

  // read config file
  char *buff = io_read_file(path, "r");
  if(!buff)
  {
    printf("Buff is NULL\n");

    return 0;
  }

  // copy contents
  char str[strlen(buff)];
  strcpy(str, buff);

  // count tokens
  int t_count = 0;
  char *token = strtok(str, "=\n");

  // keep tokenizing
  while(token)
  {
    // if a token is an item header then
    // tokenize again and go to next loop
    if(!strcmp(token, ITEM_HEADER))
    {
      token = strtok(NULL, "=\n");

      continue;
    }

    t_count++;
    token = strtok(NULL, "=\n");
  }

  // should be divisible by two
  // otherwise we are missing either a key or a value
  // from the config file
  if(t_count % 2)
  {
    printf("Syntax error in config file %s\n", path);
    printf("Config is missing a key or value\n");
    free(buff);

    return 0;
  }

  // malloc space for key=value pairs
  conf->key_value_pair_count = t_count / 2;
  conf->vars = malloc(sizeof(conf_var_t) * conf->key_value_pair_count);

  // i = index
  // t == 0, key
  // t == 1, value
  int i = 0;
  int t = 0;

  // copy buff again since it got mangled by the strtok calls
  strcpy(str, buff);

  // now points to the first key
  token = strtok(str, "=\n");

  // keep tokenizing
  while(token)
  {
    // if a token is an item header then
    // tokenize again and go to next loop
    if(!strcmp(token, ITEM_HEADER))
    {
      token = strtok(NULL, "=\n");

      continue;
    }

    // it's a key
    if(!t)
    {
      strcpy(conf->vars[i].key, token);
    }
    // it's a value
    else
    {
      if(is_number(token))
      {
        // store str converted into an int
        conf->vars[i].conf_var_u.i = atoi(token);
        conf->vars[i].type = conf_type_int;
      }
      else if(token[0] == 'I' && token[1] == 'D')
      {
        conf->vars[i].conf_var_u.i = id_lookup(token);;
        conf->vars[i].type = conf_type_int;
      }
      else if(token[0] == 'T' && token[1] == 'Y' && token[2] == 'P' && token[3] == 'E')
      {
        conf->vars[i].conf_var_u.i = type_lookup(token);;
        conf->vars[i].type = conf_type_int;
      }
      else
      {
        // store str
        strcpy(conf->vars[i].conf_var_u.s, token);
        conf->vars[i].type = conf_type_string;
      }

      // next index
      i++;
    }

    // switch between a key and a value
    t = !t;

    // keep tokenizing the str
    token = strtok(NULL, "=\n");
  }

  // mark success
  conf->success = 1;

  #if DEBUG
  printf("\nConfig vars:\nkey_value_pair_count: %d\n\n", conf->key_value_pair_count);

  for(int i = 0 ; i < conf->key_value_pair_count; i++)
  {
    if (i == conf->key_value_pair_count / 2)
    {
      printf("\n");
    }

    printf("%s = ", conf->vars[i].key);

    switch (conf->vars[i].type)
    {
      case conf_type_int:
      {
        printf("%d\n", conf->vars[i].conf_var_u.i);
      } break;

      case conf_type_string:
      {
        printf("%s\n", conf->vars[i].conf_var_u.s);
      } break;
    }
  }

  printf("\n");
  #endif

  free(buff);
  buff = NULL;

  return 1;
}

// [free the malloc'd conf_t pointer]
// 
// [conf] [conf_t pointer]
static void conf_free(conf_t *conf)
{
  // return if not valid
  if(!conf->success)
  {
    return;
  }

  // otherwise, free
  free(conf->vars);
  conf = NULL;
}

#endif // UTIL_CONF_H