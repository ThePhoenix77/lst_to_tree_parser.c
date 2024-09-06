#include "../includes/minishell.h"

/*typedef enum e_type_node
{
	PiPe,
	CMD,
}	t_type_node;

typedef struct e_redir
{
	struct	e_redir *next;
	char	*file_name;
	t_type	type;
}	t_redir;

typedef struct s_tree
{
	t_type_node		type;
	struct s_tree	*left;
	t_redir			*head_redir;
	struct s_tree	*right;
	char			**cmd_args;
	t_fd			fd;
}	t_tree;
*/

void *ft_realloc(void *ptr, int ols_s, int new_s)
{
    void *new_ptr;

    if (new_s == 0)
    {
        free(ptr);
        return (NULL);
    }
    new_ptr = malloc(new_s);
    if (!new_ptr)
    {
        perror("malloc");
        // exit(EXIT_FAILURE);
    }
    if (ptr)
    {
        ft_memcpy(new_ptr, ptr, ols_s);
        free(ptr);
    }
    return (new_ptr);
}

t_tree *create_tree_node(t_type_node type)
{
    t_tree *node;
    
    node = malloc(sizeof(t_tree));
    if (!node)
    {
        perror("malloc");
        // exit(EXIT_FAILURE);
    }
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->head_redir = NULL;
    node->cmd_args = NULL;
    node->fd.in = -1;
    node->fd.out = -1;
    return (node);
}

void add_redirection(t_tree *node, t_redir *redir)
{
    t_redir *current;
    
    current = node->head_redir;
    if (current == NULL)
        node->head_redir = redir;
    else
    {
        while (current->next)
            current = current->next;
        current->next = redir;
    }
}

void add_cmd_args(t_tree *node, char **args)
{
    node->cmd_args = args;
}

void free_tree(t_tree *tree)
{
    t_redir *redir;
    t_redir *tmp;
    int i = 0;

    if (!tree)
        return;
    free_tree(tree->left);
    free_tree(tree->right);
    redir = tree->head_redir;
    while (redir)
    {
        tmp = redir;
        redir = redir->next;
        free(tmp->file_name);
        free(tmp);
    }
    if (tree->cmd_args)
    {
        while (tree->cmd_args[i])
            free(tree->cmd_args[i++]);
        free(tree->cmd_args);
    }
    free(tree);
}

void handle_word(char ***cmd_args, int *cmd_arg_count, t_lst *tokens)
{
    *cmd_args = ft_realloc(*cmd_args, (*cmd_arg_count) * sizeof(char *),
                           (*cmd_arg_count + 1) * sizeof(char *));
    (*cmd_args)[(*cmd_arg_count)++] = ft_strdup(tokens->content);
}
void handle_pipe(char ***cmd_args, int *cmd_arg_count, t_tree **last_cmd)
{
    t_tree *pipe_node;
    
    pipe_node = create_tree_node(PiPe);
    *cmd_args = ft_realloc(*cmd_args, (*cmd_arg_count) * sizeof(char *),
                           (*cmd_arg_count + 1) * sizeof(char *));
    (*cmd_args)[*cmd_arg_count] = NULL;
    add_cmd_args(*last_cmd, *cmd_args);
    pipe_node->left = *last_cmd;
    (*last_cmd)->right = pipe_node;
    *last_cmd = pipe_node;
    *cmd_args = NULL;
    *cmd_arg_count = 0;
}

int is_redirection(int type)
{
    return (type == REDIR_OUT || type == REDIR_IN ||
            type == HERE_DOC || type == DREDIR_OUT);
}

void handle_redir(t_lst *tokens, t_tree *last_cmd, t_redir **curr_redir)
{
    if (*curr_redir == NULL)
    {
        *curr_redir = malloc(sizeof(t_redir));
        (*curr_redir)->type = tokens->type;
        (*curr_redir)->file_name = NULL;
        (*curr_redir)->next = NULL;
        add_redirection(last_cmd, *curr_redir);
    }
    if (tokens->next && tokens->next->type == WORD)
    {
        (*curr_redir)->file_name = ft_strdup(tokens->next->content);
    }
}

void finalize_command(t_tree *last_cmd, char ***cmd_args, int cmd_arg_count)
{
    *cmd_args = ft_realloc(*cmd_args, cmd_arg_count * sizeof(char *),
                           (cmd_arg_count + 1) * sizeof(char *));
    (*cmd_args)[cmd_arg_count] = NULL;
    add_cmd_args(last_cmd, *cmd_args);
}

t_tree *parse_to_tree(t_lst *tokens)
{
    t_tree *root;
    t_tree *last_cmd;
    t_redir *curr_redir;
    char **cmd_args;
    int cmd_arg_count;

    root = create_tree_node(CMD);
    last_cmd = root;
    curr_redir = NULL;
    cmd_args = NULL;
    cmd_arg_count = 0;
    while (tokens)
    {
        if (tokens->type == WORD)
            handle_word(&cmd_args, &cmd_arg_count, tokens);
        else if (tokens->type == PiPe)
            handle_pipe(&cmd_args, &cmd_arg_count, &last_cmd);
        else if (is_redirection(tokens->type))
            handle_redir(tokens, last_cmd, &curr_redir);
        tokens = tokens->next;
    }
    finalize_command(last_cmd, &cmd_args, cmd_arg_count);
    return (root);
}
