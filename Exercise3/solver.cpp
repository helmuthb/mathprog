#include <iostream>
#include <ilcplex/ilocplex.h>
#include <vector>
#include <list>

using namespace std;

ILOSTLBEGIN


class SportsLeague {
public:
  int n;
  int k;
  IloEnv env;
  IloCplex cplex;
  IloModel model;
  IloExpr target;
  IloBoolVarArray gameWon;
  IloBoolVarArray gameLost;
  IloBoolVarArray gameDraw;
  int pos(int i, int j);
  SportsLeague(int n, int k);
  void getVars();
  void getStatus();
  void solve();
};

// helper function: calculating the index in the
// one-dimensional array for (i,j)
int SportsLeague::pos(int i, int j) {
  return i*(n-1) + j - (j>i?1:0);
}

SportsLeague::SportsLeague(int n, int k) : n(n), k(k), env(), cplex(env),
    model(env), target(env),
    gameWon(env, n*(n-1)), gameLost(env, n*(n-1)), gameDraw(env, n*(n-1)) {
  try {
    // define variables
    char varName[16];
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        if (i == j) continue;
        sprintf(varName, "w(%d,%d)", i, j);
        gameWon[pos(i,j)] = IloBoolVar(env, varName);
        sprintf(varName, "l(%d,%d)", i, j);
        gameLost[pos(i,j)] = IloBoolVar(env, varName);
        sprintf(varName, "d(%d,%d)", i, j);
        gameDraw[pos(i,j)] = IloBoolVar(env, varName);
      }
    }
    // add constraints
    // 1. w_ij + l_ij + d_ij == 1
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        if (i == j) continue;
        IloExpr sumone(env);
        sumone += gameWon[pos(i,j)];
        sumone += gameLost[pos(i,j)];
        sumone += gameDraw[pos(i,j)];
        model.add(sumone == 1);
        sumone.end();
      }
    }
    // 2. 3w_ij + d_ij + d_ji + 3l_ji >= (... the same for i-1)
    for (int i=1; i<n; i++) {
      for (int j=0; j<n; j++) {
        if (i == j) continue;
        IloExpr higher(env);
        IloExpr lower(env);
        higher += 3*gameWon[pos(i,j)];
        higher += gameDraw[pos(i,j)];
        higher += gameDraw[pos(j,i)];
        higher += 3*gameLost[pos(j,i)];
        lower += 3*gameWon[pos(i-1,j)];
        lower += gameDraw[pos(i-1,j)];
        lower += gameDraw[pos(j,i-1)];
        lower += 3*gameLost[pos(j,i-1)];
        model.add(higher >= lower);
        higher.end();
        lower.end();
      }
    }
    // Add target function
    target += 1;
    for (int j=0; j<n; j++) {
      if (j == (k-1)) continue;
      IloExpr fromJ(env);
      fromJ += 3*gameWon[pos(k-1,j)];
      fromJ += gameDraw[pos(k-1,j)];
      fromJ += gameDraw[pos(j,k-1)];
      fromJ += 3*gameLost[pos(j,k-1)];
      target += fromJ;
      fromJ.end();
    }
    model.add(IloMaximize(env, target));
  }
  catch(IloException& e) {
    cerr << e;
  }
}

void SportsLeague::solve() {
  cplex.extract(model);
  cplex.solve();
}

void SportsLeague::getStatus() {
  IloAlgorithm::Status algStatus = cplex.getStatus();
  if (algStatus != IloAlgorithm::Optimal) {
    cerr << "Could not find optimal solution!" << endl;
  }
  else {
    cout << "OPT: " << cplex.getObjValue() << endl;
  }
}

void SportsLeague::getVars() {
  for (int i=0; i<n; i++) {
    int points = 0;
    for (int j=0; j<n; j++) {
      if (i == j) {
        cout << " ";
        continue;
      }
      if (cplex.getValue(gameWon[pos(i,j)])) {
        cout << "3";
        points += 3;
      }
      if (cplex.getValue(gameLost[pos(i,j)])) {
        cout << "0";
      }
      if (cplex.getValue(gameDraw[pos(i,j)])) {
        cout << "1";
        points += 1;
      }
    }
    cout << " - ";
    for (int j=0; j<n; j++) {
      if (i == j) {
        cout << " ";
        continue;
      }
      if (cplex.getValue(gameLost[pos(j,i)])) {
        cout << "3";
        points += 3;
      }
      if (cplex.getValue(gameWon[pos(j,i)])) {
        cout << "0";
      }
      if (cplex.getValue(gameDraw[pos(j,i)])) {
        cout << "1";
        points += 1;
      }
    }
    cout << " ... " << points << endl;
  }
}

int main(int agrc, char* argv[]) {
  SportsLeague league(18, 3);
  league.solve();
  league.getStatus();
  league.getVars();
}
