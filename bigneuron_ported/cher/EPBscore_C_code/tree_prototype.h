/* tree.c */
void set_stencil(int nx, int nxy, int *sten);
void set_2D_restricted_stencil(int nx, int nxy, int *sten);
NODE *set_child_node(int ind, unsigned char burn_num, NODE *parent);
short num_children(int ind, short *pma, int *stencil);
void add_children_to_tree(NODE *par, short *pma, unsigned char *burn_dat, int *stencil);
NODE *make_BF_tree(short *pma, unsigned char *burn_dat, int rind, int *stencil, int NXY, int NX);
TREE_LIST *set_pma_trees(short *pma, unsigned char *burn_dat, int nx, int nxy, int nxyz);
void print_node(NODE *nod, int NXY, int NX);
void print_level_nodes(short num_nodes, NODE **node_list, int NXY, int NX);
void print_tree_BF(NODE *root, int NXY, int NX);
void print_TREE_LIST(TREE_LIST *trees, int NXY, int NX);
void print_TREE_LIST_info(TREE_LIST *trees);
void echo_int_list(int n, int *list, char *name);
void compress_int_list(int *n, int *ulist, int *clist);
int BF_tree_count_nodes(NODE *root);
int BF_tree_limits(NODE *root, int *ix_min, int *ix_max, int *iy_min, int *iy_max, int *iz_min, int *iz_max, int NXY, int NX);
void tree_2_pma_driver(TREE_LIST *tree_list, short *pma);
void tree_2_pma(NODE *root, short *pma);
int BF_trim_tree_count_nodes(NODE *root, short *pma);
void free_tree_list(TREE_LIST *tl_ptr);
void free_tree(TREE_LIST *t_ptr);
void free_node(NODE *n_ptr);