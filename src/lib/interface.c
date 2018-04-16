#include "interface.h"
#include "common.h"
#include "parser.h"
#include "users.h"
#include "post.h"
#include "date.h"
#include "tcd.h"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <time.h>

TAD_community init()
{
    return init_tcd();
}
// lixo
void func(gpointer data,gpointer user_data){
    printf("id: %ld nr de posts: %d\n",get_id_user(data),get_nr_posts_user(data));
}

static gint comp_nr_posts(gconstpointer a,gconstpointer b){
    if(get_nr_posts_user((ptr_user)a)>get_nr_posts_user((ptr_user)b)) 
        return 1;
    else if(get_nr_posts_user((ptr_user)a)<get_nr_posts_user((ptr_user)b))
        return -1;
    else 
        return 0;
}

static gboolean func_print(gpointer key,gpointer value,gpointer data){ 
    print_user(value);
    return TRUE;
}

static gboolean load_rank_gslist(gpointer key,gpointer value,gpointer data){ 
    TAD_community com = (TAD_community)GPOINTER_TO_SIZE(data);
    set_rank_n_posts(com, g_slist_prepend(get_rank_n_posts(com),value));
    return FALSE;

}
// query 0
TAD_community load(TAD_community com, char* dump_path){
    streamUsers(get_hash_users(com),dump_path);
    streamPosts(com,dump_path);
    //load lista ligada de utilizadores organizada por nº de posts
    g_hash_table_foreach(get_hash_users(com),(GHFunc)load_rank_gslist,
            GSIZE_TO_POINTER(com));
    set_rank_n_posts(com,
            g_slist_sort(get_rank_n_posts(com), comp_nr_posts));
   return com;
}  

// query 1
STR_pair info_from_post(TAD_community com, long id){

    STR_pair sp = NULL; 
    char* title = NULL;
    char* name = NULL;
    ptr_user n;
    ptr_post t = (ptr_post)g_hash_table_lookup(get_hash_posts(com),
            GSIZE_TO_POINTER(id));

    if(get_post_type_id(t)==1){
        n = (ptr_user)g_hash_table_lookup(get_hash_users(com),
                GSIZE_TO_POINTER(get_owner_user_id(t)));
        title = get_title(t);
        name = get_displayname_user(n);
    }else if(get_post_type_id(t) == 2){
        t = (ptr_post)g_hash_table_lookup(get_hash_posts(com), 
                GSIZE_TO_POINTER(get_parent_id(t)));
        n = (ptr_user)g_hash_table_lookup(get_hash_users(com),
                GSIZE_TO_POINTER(get_owner_user_id(t)));
        title = get_title(t);
        name = get_displayname_user(n);
    }

    return sp =  create_str_pair(title,name);
}

// query 2
LONG_list top_most_active(TAD_community com, int N){
    LONG_list l = create_list(N);
    int i;
    ptr_user u;
    GSList *list = get_rank_n_posts(com);
    
    for(i=0;i<N && list->next != NULL;i++){
        u =(ptr_user) GPOINTER_TO_SIZE(list->data);
        set_list(l,i,get_id_user(u));
        list = list->next;
    }
    
    for(i=0;i<N;i++){
        printf("Id do nº %d: %ld\n",i+1,get_list(l,i));
    }
    
    return l;
}

//----------------------------------------------------------------------

typedef struct date_pair{
    Date begin;
    Date end;
    int nq;
    int na;
}* limitDates;

static gboolean func_n_answer(gpointer key,gpointer value,gpointer data){
limitDates ld = (limitDates)GPOINTER_TO_SIZE(data);
    Date b = ld->begin;
    Date e = ld->end;
      
    if ((date_compare(get_creation_date(value),b)>0 && 
          date_compare(e,get_creation_date(value))>0)){

        ld->na += g_tree_nnodes(get_answer_tree(value));
    }
    return FALSE;
}

static gboolean func_q_btw(gpointer key,gpointer value,gpointer data){
    limitDates ld = (limitDates)GPOINTER_TO_SIZE(data);
    Date b = ld->begin;
    Date e = ld->end;

    if ((date_compare(get_creation_date(value),b)>0 && 
          date_compare(e,get_creation_date(value))>0)){

        ld->nq++;
    }
    return FALSE;
}

//------------------------------------------------------
// query 3
LONG_pair total_posts(TAD_community com, Date begin, Date end){
    limitDates ld = malloc(sizeof(struct date_pair));
    ld->begin = begin;
    ld->end = end;
    ld->nq = 0;
    ld->na = 0;

    g_tree_foreach(get_tree_posts(com),(GTraverseFunc)func_q_btw,
            GSIZE_TO_POINTER(ld));

    g_tree_foreach(get_tree_posts(com),(GTraverseFunc)func_n_answer,
            GSIZE_TO_POINTER(ld));

    printf("Numero de users: %d\n",g_hash_table_size(get_hash_users(com)));
    printf("Numero de respostas: %d\n",ld->na);
    printf("Numero de perguntas: %d\n",ld->nq);
    printf("Numero de posts: %d\n",ld->na+ld->nq);

    LONG_pair lp = create_long_pair(ld->nq,ld->na);
    return lp;
}

//----------------------------------------------------------------------

typedef struct tags{
    Date begin;
    Date end;
}* tagsInfo;
/*
static gboolean func_q_tags(gpointer key,gpointer value,gpointer data){
    limitDates ld = (limitDates)GPOINTER_TO_SIZE(data);
    Date b = ld->begin;
    Date e = ld->end;

    if ((date_compare(get_creation_date(value),b)>0 && 
          date_compare(e,get_creation_date(value))>0)){

         if(strstr(get_tags(value),tag) != NULL) {
            ld->nq++;
         }
    }
    return FALSE;
}*/

// query 4
LONG_list questions_with_tag(TAD_community com, char* tag, Date begin, Date end);
/*    tagsInfo ld = malloc(sizeof(struct date_pair));
    ld->begin = begin;
    ld->end = end;

    g_tree_foreach(get_tree_posts(com),(GTraverseFunc)func_q_tags,
            GSIZE_TO_POINTER(ld));

    LONG_list ll = create_long_list(5);
    return ll;*/ 

// query 5
USER get_user_info(TAD_community com, long id);

// query 6
LONG_list most_voted_answers(TAD_community com, int N, Date begin, Date end);

// query 7
LONG_list most_answered_questions(TAD_community com, int N, Date begin, Date end);

// query 8
LONG_list contains_word(TAD_community com, char* word, int N){
    LONG_list l = create_list(N);
    /*GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, com->hashPosts);
   
    while(g_hash_table_iter_next(&iter, &key, &value) && N>0){

        //TODO struct ordenada por data que não contenha apenas 1 post por dia 
    }*/
    return l;
}
       

// query 9
LONG_list both_participated(TAD_community com, long id1, long id2, int N);

// query 10
long better_answer(TAD_community com, long id);

// query 11
LONG_list most_used_best_rep(TAD_community com, int N, Date begin, Date end);

TAD_community clean(TAD_community com);
