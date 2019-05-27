#ifndef UTIL_CONF_H
#define UTIL_CONF_H

#define ITEM_HEADER "[ITEM]"
#define KEY_VALUE_PAIRS_PER_ITEM 9

internal char*
type_lookup_table[] =
{
  "TYPE_CONSUME",
  "TYPE_EQUIP"
};

enum
{
  token_type_key,
  token_type_value,
} token_type_e;

typedef enum
{
  data_type_int,
  data_type_string
} data_type_e;

typedef struct
{
  char key[32];
  data_type_e type;
  union
  {
    i32 i;
    char str[256];
  } conf_var_u;
} var_t;

typedef struct
{
  var_t *vars;
  i32 key_value_pair_count;
  b32 success;
} conf_t;

// [checks if token can be found from the lookup table]
// 
// [token] [string token to check]
// 
// [returns the number of the token if found]
// [returns -1 if token was not found]
internal i32
type_lookup(char *token)
{
  i32 result = 0;

  for(i32 i = 0; i < type_count; i++)
  {
    if(str_cmp(token, type_lookup_table[i]))
    {
      result = i + 1;
      break;
    }
  }

  return result;
}

// [checks if character is one of the standard white-space characters]
// 
// [ch] [character to check]
// 
// [returns 1 for true]
// [returns 0 for false]
internal i32
is_space(i32 ch)
{
  i32 result = 0;

  if(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r')
  {
    result = 1;
  }

  return result;
}

// [checks if the given string is a number or not]
// 
// [str] [string to check]
// 
// [returns 1 for true]
// [returns 0 for false]
internal i32
is_number(char *str)
{
  // Handle cases where the pointer is NULL,
  // the character pointed to is a null-terminator
  // or one of the standard white-space characters
  if(str == NULL || *str == '\0' || is_space(*str))
  {
    return 0;
  }

  char *p;
  strtod(str, &p);

  // if the char is a number, *p will equal to '\0' and the return value is 1
  // if the char is a character, *p will equal to the first letter instead of '\0' and the return value is 0
  return *p == '\0';
}

// [load conf file into a conf_t struct]
// 
// [path] [path to the file]
internal conf_t*
load_conf(char *path)
{
  debug("Loading config file %s\n", path);

  conf_t *conf = malloc(sizeof(conf_t));
  conf->success = false;

  // read config file
  char *buff = read_file(path, "r");
  if(!buff)
  {
    debug("Could not load config\n");
    return NULL;
  }

  // copy contents
  char str[strlen(buff)];
  strcpy(str, buff);

  // count tokens
  i32 t_count = 0;
  char *token = strtok(str, "=\n");

  // keep tokenizing
  while(token)
  {
    // if a token is an item header then
    // tokenize again and go to next loop
    if(str_cmp(token, ITEM_HEADER))
    {
      token = strtok(NULL, "=\n");
      continue;
    }

    t_count++;
    token = strtok(NULL, "=\n");
  }

  debug("token count: %d\n", t_count);

  // not equal amount of keys and variables
  if(t_count % 2)
  {
    debug("Syntax error in config file %s\n\n", path);
    debug("Config is missing a key or value\n\n");

    free(buff);
    return NULL;
  }
  // not enough key value pairs per item
  else if(t_count % KEY_VALUE_PAIRS_PER_ITEM)
  {
    debug("Syntax error in config file %s\n\n", path);
    debug("One or more items have missing or excess information\n\n");

    free(buff);
    return NULL;
  }

  // malloc space for key=value pairs
  conf->key_value_pair_count = t_count / 2;
  conf->vars = malloc(sizeof(var_t) * conf->key_value_pair_count);

  i32 i = 0;
  i32 t = token_type_key;

  // copy buff again since it got mangled by strtok
  strcpy(str, buff);

  // points to the first key
  token = strtok(str, "=\n");

  while(token)
  {
    if(str_cmp(token, ITEM_HEADER))
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
      // it's a number value
      if(is_number(token))
      {
        // store str converted into an int
        conf->vars[i].conf_var_u.i = atoi(token);
        conf->vars[i].type = data_type_int;
      }
      // it's a specific string
      else if(token[0] == 'I' && token[1] == 'D')
      {
        conf->vars[i].conf_var_u.i = atoi(token);
        conf->vars[i].type = data_type_int;
      }
      else if(token[0] == 'T' && token[1] == 'Y' && token[2] == 'P' && token[3] == 'E')
      {
        conf->vars[i].conf_var_u.i = type_lookup(token);
        conf->vars[i].type = data_type_int;
      }
      // it's a general string
      else
      {
        strcpy(conf->vars[i].conf_var_u.str, token);
        conf->vars[i].type = data_type_string;
      }

      i++;
    }

    t = !t;
    token = strtok(NULL, "=\n");
  }

  conf->success = true;

  debug("key_value_pair_count: %d\n", conf->key_value_pair_count);

  for(i32 i = 0 ; i < conf->key_value_pair_count; i++)
  {
    debug("%s = ", conf->vars[i].key);

    if(conf->vars[i].type == data_type_int)
    {
        debug("%d\n", conf->vars[i].conf_var_u.i);
    }
    else if(conf->vars[i].type == data_type_string)
    {
        debug("%s\n", conf->vars[i].conf_var_u.str);
    }
  }

  free(buff);
  buff = NULL;

  debug("Config %s loaded\n\n", path);
  return conf;
}

// [free the malloc'd conf_t pointer]
// 
// [conf] [conf_t pointer]
//
// [returns 1 for success]
// [returns 0 for failure]
internal i32
free_conf(conf_t *conf)
{
  i32 result = 0;

  if(conf)
  {
    if(conf->vars)
    {
      free(conf->vars);
      conf->vars = NULL;
    }

    free(conf);
    conf = NULL;

    result = 1;
  }

  return result;
}

#endif // UTIL_CONF_H