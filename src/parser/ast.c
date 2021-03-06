#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FilterNode* NewVaryingPredicateNode(const char* lAlias, const char* lProperty, int op, const char* rAlias, const char* rProperty) {
	FilterNode *n = malloc(sizeof(FilterNode));
	n->t = N_PRED;

	n->pn.t = N_VARYING;
	n->pn.alias = (char*)malloc(strlen(lAlias) + 1);
	n->pn.property = (char*)malloc(strlen(lProperty) + 1);
	n->pn.nodeVal.alias = (char*)malloc(strlen(rAlias) + 1);
	n->pn.nodeVal.property = (char*)malloc(strlen(rProperty) + 1);

	strcpy(n->pn.alias, lAlias);
	strcpy(n->pn.property, lProperty);
	strcpy(n->pn.nodeVal.alias, rAlias);
	strcpy(n->pn.nodeVal.property, rProperty);

	n->pn.op = op;

	return n;
}

FilterNode* NewConstantPredicateNode(const char* alias, const char* property, int op, SIValue value) {
	FilterNode *n = malloc(sizeof(FilterNode));
  	n->t = N_PRED;

	n->pn.t = N_CONSTANT;
  	n->pn.alias = strdup(alias);
	n->pn.property = strdup(property);

	n->pn.op = op;
	n->pn.constVal = value;

	return n;
}


FilterNode *NewConditionNode(FilterNode *left, int op, FilterNode *right) {
  FilterNode *n = malloc(sizeof(FilterNode));
  n->t = N_COND;
  n->cn.left = left;
  n->cn.right = right;
  n->cn.op = op;

  return n;
}


void FreePredicateNode(PredicateNode* predicateNode) {

	if(predicateNode->alias) {
		free(predicateNode->alias);
	}

	if(predicateNode->property) {
		free(predicateNode->property);
	}

	if(predicateNode->t == N_VARYING) {
		if(predicateNode->nodeVal.alias) {
			free(predicateNode->nodeVal.alias);
		}

		if(predicateNode->nodeVal.property) {
			free(predicateNode->nodeVal.property);
		}
	}

	// TODO: Should I free constVal?
}


void FreeFilterNode(FilterNode* filterNode) {
	if(!filterNode)
		return;

	switch(filterNode->t) {
		case N_PRED:
			FreePredicateNode(&filterNode->pn);
			break;
		case N_COND:
			FreeFilterNode(filterNode->cn.left);
			FreeFilterNode(filterNode->cn.right);
			break;
	}
}

GraphEntity* NewLinkEntity(char *alias, char *label, Vector *properties, LinkDirection dir) {
	LinkEntity* le = (LinkEntity*)calloc(1, sizeof(LinkEntity));
	le->direction = dir;
	le->ge.t = N_LINK;
	le->ge.properties = properties;
	
	if(label != NULL) {
		le->ge.label = strdup(label);
	}
	if(alias != NULL) {
		le->ge.alias = strdup(alias);
	}

	return le;
}

GraphEntity* NewNodeEntity(char *alias, char *label, Vector *properties) {
	NodeEntity* ge = (NodeEntity*)calloc(1, sizeof(NodeEntity));
	ge->t = N_ENTITY;
	ge->properties = properties;
	
	if(alias != NULL) {
		ge->alias = strdup(alias);
	}
	if(label != NULL) {
		ge->label = strdup(label);
	}

	return ge;
}

void FreeGraphEntity(GraphEntity* graphEntity) {
	if(graphEntity->label != NULL) {
		free(graphEntity->label);
	}
	if(graphEntity->alias != NULL) {
		free(graphEntity->alias);
	}
	if(graphEntity->properties != NULL) {
		for(int i = 0; i < Vector_Size(graphEntity->properties); i++) {
			SIValue *val;
			Vector_Get(graphEntity->properties, i, &val);
			SIValue_Free(val);
			free(val);
		}
		Vector_Free(graphEntity->properties);
	}
	free(graphEntity);
}

MatchNode* NewMatchNode(Vector* elements) {
	MatchNode* matchNode = (MatchNode*)malloc(sizeof(MatchNode));
	matchNode->graphEntities = elements;

	return matchNode;
}

void FreeMatchNode(MatchNode* matchNode) {
	for(int i = 0; i < Vector_Size(matchNode->graphEntities); i++) {
		GraphEntity* ge;
		Vector_Get(matchNode->graphEntities, i, &ge);
		FreeGraphEntity(ge);
	}

	Vector_Free(matchNode->graphEntities);
	free(matchNode);
}


WhereNode* NewWhereNode(FilterNode* filters) {
	WhereNode* whereNode = (WhereNode*)malloc(sizeof(WhereNode));
	whereNode->filters = filters;
	return whereNode;
}

void FreeWhereNode(WhereNode* whereNode) {
	if(!whereNode) {
		return;
	}

	FreeFilterNode(whereNode->filters);
	free(whereNode);
}


ReturnNode* NewReturnNode(Vector* returnElements, int distinct) {
	ReturnNode* returnNode = (ReturnNode*)malloc(sizeof(ReturnNode));
	returnNode->returnElements = returnElements;
	returnNode->distinct = distinct;
	return returnNode;
}

void FreeReturnNode(ReturnNode* returnNode) {
	for (int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
		ReturnElementNode *node;
		Vector_Get(returnNode->returnElements, i, &node);
		FreeReturnElementNode(node);
	}
	
	Vector_Free(returnNode->returnElements);
	free(returnNode);
}

ReturnElementNode* NewReturnElementNode(ReturnElementType type, Variable* variable, const char* aggFunc, const char* alias) {
	ReturnElementNode* returnElementNode = (ReturnElementNode*)malloc(sizeof(ReturnElementNode));
	returnElementNode->type = type;
	returnElementNode->variable = variable;
	returnElementNode->func = NULL;
	returnElementNode->alias = NULL;

	if(type == N_AGG_FUNC) {
		returnElementNode->func = strdup(aggFunc);
	}

	if(alias != NULL) {
		returnElementNode->alias = strdup(alias);
	}
	
	return returnElementNode;
}

void FreeReturnElementNode(ReturnElementNode* returnElementNode) {
	if(returnElementNode != NULL) {
		FreeVariable(returnElementNode->variable);

		if(returnElementNode->type == N_AGG_FUNC) {
			free(returnElementNode->func);
		}
		if(returnElementNode->alias != NULL) {
			free(returnElementNode->alias);
		}

		free(returnElementNode);
	}
}


QueryExpressionNode* NewQueryExpressionNode(MatchNode* matchNode, WhereNode* whereNode, ReturnNode* returnNode, OrderNode* orderNode, LimitNode* limitNode) {
	QueryExpressionNode* queryExpressionNode = (QueryExpressionNode*)malloc(sizeof(QueryExpressionNode));
	
	queryExpressionNode->matchNode = matchNode;
	queryExpressionNode->whereNode = whereNode;
	queryExpressionNode->returnNode = returnNode;
	queryExpressionNode->orderNode = orderNode;
	queryExpressionNode->limitNode = limitNode;

	return queryExpressionNode;
}

void FreeQueryExpressionNode(QueryExpressionNode* queryExpressionNode) {
	FreeMatchNode(queryExpressionNode->matchNode);
	FreeWhereNode(queryExpressionNode->whereNode);
	FreeReturnNode(queryExpressionNode->returnNode);
	FreeOrderNode(queryExpressionNode->orderNode);
	free(queryExpressionNode);
}

Variable* NewVariable(const char* alias, const char* property) {
	Variable* v = (Variable*)calloc(1, sizeof(Variable));

	if(alias != NULL) {
		v->alias = strdup(alias);
	}
	if(property != NULL) {
		v->property = strdup(property);
	}

	return v;
}

void FreeVariable(Variable* v) {
	if(v != NULL) {
		if(v->alias != NULL) {
			free(v->alias);
		}
		if(v->property != NULL) {
			free(v->property);
		}
		free(v);
	}
}

OrderNode* NewOrderNode(Vector* columns, OrderByDirection direction) {
	OrderNode* orderNode = (OrderNode*)malloc(sizeof(OrderNode));
	orderNode->columns = columns;
	orderNode->direction = direction;
	return orderNode;
}

void FreeOrderNode(OrderNode* orderNode) {
	if(orderNode != NULL) {
		for(int i = 0; i < Vector_Size(orderNode->columns); i++) {
			ColumnNode* c = NULL;
			Vector_Get(orderNode->columns, i , &c);
			FreeColumnNode(c);
		}

		Vector_Free(orderNode->columns);
		free(orderNode);
	}
}

ColumnNode* NewColumnNode(const char* alias, const char* property, ColumnNodeType type) {
	ColumnNode* node = malloc(sizeof(ColumnNode));
	node->type = type;
	node->alias = NULL;
	node->property = NULL;

	node->alias = malloc(sizeof(char) * (strlen(alias) + 1));
	strcpy(node->alias, alias);

	if(type == N_VARIABLE) {
		node->property = malloc(sizeof(char) * (strlen(property) + 1));
		strcpy(node->property, property);
	}

	return node;
}

ColumnNode* ColumnNodeFromVariable(const Variable* variable) {
	return NewColumnNode(variable->alias, variable->property, N_VARIABLE);
}

ColumnNode* ColumnNodeFromAlias(const char* alias) {
	return NewColumnNode(alias, NULL, N_ALIAS);
}

void FreeColumnNode(ColumnNode* node) {
	if(node != NULL) {
		if(node->alias != NULL) {
			free(node->alias);
		}
		if(node->type == N_VARIABLE && node->property != NULL) {
			free(node->property);
		}
		free(node);
	}
}

LimitNode* NewLimitNode(int limit) {
	LimitNode* limitNode = (LimitNode*)malloc(sizeof(limitNode));
	limitNode->limit = limit;
	return limitNode;
}

void FreeLimitNode(LimitNode* limitNode) {
	if(limitNode) {
		free(limitNode);
	}
}