

struct node {
	unsigned int value;
	unsigned int options;
	unsigned int numoptions;
	int changed;
	struct nodegrp *grps[3];
};

struct nodegrp {
	struct node *members[9];
	unsigned int indices[9];
	unsigned int value_flag;
};

struct sudoku {
	struct node nodes[81];
	struct nodegrp rows[9];
	struct nodegrp cols[9];
	struct nodegrp blks[9];
	int nodes_left;
	int nodes_changed;
};
