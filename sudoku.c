#include <stdio.h>
#include <stdlib.h>

#include "sudoku.h"

#define OPTION(n)	(0x1 << (n - 1))

static inline int get_value(unsigned int options)
{
	int val = 0;
	while (options) {
		val++;
		options /= 2 ;
	}
	return val;
}

void initialize_sudoku(struct sudoku *p)
{
	int r, c, b, bi;

	int n;

	for (n = 0; n < 81; n++) {
		r = n / 9;
		c = n % 9;
		b = (r / 3) * 3 + (c / 3);
		bi = (r % 3) * 3 + (c % 3);

		p->rows[r].members[c] = p->nodes + n;
		p->rows[r].indices[c] = n;

		p->cols[c].members[r] = p->nodes + n;
		p->cols[c].indices[r] = n;

		p->blks[b].members[bi] = p->nodes + n;
		p->blks[b].indices[bi] = n;

		p->nodes[n].grps[0] = p->rows + r;
		p->nodes[n].grps[1] = p->cols + c;
		p->nodes[n].grps[2] = p->blks + b;
	}
}

void update_node_with_value(struct sudoku *p, struct node *node, int val)
{
	if (val) {
		node->value = val;
		node->options = OPTION(val);
		node->numoptions = 1;
		node->changed = 3;
		p->nodes_left--;
		p->nodes_changed++;
	} else {
		node->value = 0;
		node->options = 0x1ff;
		node->numoptions = 9;
		node->changed = 0;
	}
}

/* Returns number of unsolved nodes */
void read_sudoku(struct sudoku *p)
{
	int i;
	int val;
	p->nodes_left = 81;
	p->nodes_changed = 0;

	for (i = 0; i < 81; i++) {
		scanf("%d", &val);
		update_node_with_value(p, p->nodes + i, val);
	}
}

void print_sudoku(struct sudoku *p)
{
	int i, j, n;

	printf("Current Sudoku\n");
	for (i = 0; i < 9; i++) {
		if (i % 3 == 0)
			printf("\n");
		for (j = 0; j < 9; j++) {
			n = i * 9 + j;
			if (j % 3 == 0)
				printf("\t");
			printf("%d ", p->nodes[n].value);
			//printf("%d ", p->nodes[n].changed);
			//printf("%d ", p->nodes[n].numoptions);
			//printf("%3x ", p->nodes[n].options);
		}
		printf("\n");
	}

	printf("Rows\t\t\t\tCols\t\t\t\tBlocks\n");
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++)
			printf("%3x ", p->rows[i].members[j]->options);
		printf("\t");
		for (j = 0; j < 9; j++)
			printf("%3x ", p->cols[i].members[j]->options);
		printf("\t");
		for (j = 0; j < 9; j++)
			printf("%3x ", p->blks[i].members[j]->options);
		printf("\n");
	}
	printf("\n");

}

int check_grp_sanity(struct nodegrp *grp)
{
	int i;
	int val;

	for (i = 0; i < 9; i++) {
		val = grp->members[i]->value;
		if (val)
			if (grp->value_flag & OPTION(val))
				return -1;
			else
				grp->value_flag |= OPTION(val);
	}
	return 0;
}

int check_data_sanity(struct sudoku *p)
{
	struct nodegrp *grp;
	int i, n;
	int val;

	for (n = 0; n < 9; n++) {
		if (check_grp_sanity(&p->rows[n])) {
			printf("Invalid data in row %d\n", n);
			return -1;
		}
			
		if (check_grp_sanity(&p->cols[n])) {
			printf("Invalid data in col %d\n", n);
			return -1;
		}
			
		if (check_grp_sanity(&p->blks[n])) {
			printf("Invalid data in blks %d\n", n);
			return -1;
		}
	}
}

void update_grp_only_value(struct sudoku *p, struct nodegrp *grp, int val)
{
	int m;
	struct node *node;

	for (m = 0; m < 9; m++) {
		node = grp->members[m];
		if (node->value)
			continue;

		if ((node->options & OPTION(val)) == 0)
			continue;

		node->options &= ~OPTION(val);
		node->numoptions--;
		if (node->numoptions == 1) {
			node->value = get_value(node->options);
			p->nodes_left--;
			printf("(1)(%2d) val (%d), options(%3x)\n",
					grp->indices[m],
					node->value,
					node->options);
			node->changed = 3;
			p->nodes_changed++;
		}
	}
}

void update_grp_only_node(struct sudoku *p, struct nodegrp *grp)
{
	int val_index;
	int val;
	int valopt;
	int n;
	struct node *node;

	/* Find if a number is possible at a particular node */
	for (val = 1; val <= 9; val++) {
		valopt = 0;
		if (grp->value_flag & OPTION(val))
			continue;
		for (n = 0; n < 9; n++) {
			node = grp->members[n];
			if (node->options & OPTION(val)) {
				if (valopt++)
					break;
				else
					val_index = n;
			}
		}
		if (valopt == 1) {
			node = grp->members[val_index];
			update_node_with_value(p, grp->members[val_index], val);
			printf("(2)(%2d) val (%d), options(%3x)\n",
					grp->indices[val_index],
					node->value,
					node->options);
		}
	}
}

void scan_grp(struct sudoku *p, struct nodegrp *grp)
{
	int n, m;
	int val;
	int options;
	struct node *node;

	for (n = 0; n < 9; n++) {
		if (grp->members[n]->changed == 0)
			continue;

		grp->members[n]->changed--;

		if (grp->members[n]->changed == 0)
			p->nodes_changed--;

		val = grp->members[n]->value;

		update_grp_only_value(p, grp, val);

		update_grp_only_node(p, grp);
	}
}

void scan_grps(struct sudoku *p, struct nodegrp **grps, int index)
{
	int n;
	
	for (n = 0; n < 3; n++)
		scan_grp(p, p->nodes[index].grps[n]);
}

int main(void)
{
	struct sudoku sudoku_data;
	struct sudoku *p;
	int n;
	int opt = 2;

	p = (struct sudoku *) malloc(sizeof(struct sudoku));
	if (!p) {
		printf("Not enough memory. Malloc failed\n");
		return -1;
	}

	initialize_sudoku(p);

	read_sudoku(p);

	if (check_data_sanity(p))
		return -1;
	print_sudoku(p);

	while (p->nodes_changed) {
		for (n = 0; n < 81; n++) {
			if (p->nodes[n].changed)
				scan_grps(p, p->nodes[n].grps, n);
		}
	};
	printf("Nodes changed = %d, left = %d\n", p->nodes_changed,
							p->nodes_left);
	print_sudoku(p);
}
