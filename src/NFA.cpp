#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>

static void *processString(void* arg);

using namespace std;

class NFA {
private:
  class Transition {
  private:
    char inputSymbol;
    string nextState;

  public:
    Transition(char inputSymbol, string nextState) {
      this->inputSymbol = inputSymbol;
      this->nextState = nextState;
    }

    string toString() {
      string retString = "(";
      retString += inputSymbol;
      retString += "->" + nextState + ")";
      return retString;
    }
  };
  //Private class that represents a state object.
  class State {
  //Private class for representing transitions.
  private:
    string stateName;
    vector<Transition> transitions;
  public:
    State(string stateName) {
        this->stateName = stateName;
    }

    State() {

    }

    void addTransition(char inputSymbol, string nextState) {
      Transition t = Transition(inputSymbol, nextState);
      transitions.push_back(t);
    }

    vector<Transition> getTransitions() {
      return transitions;
    }

    string getName() {
      return stateName;
    }

    //Get a string representation of this state for display
    //purposes.
    string toString() {
      string retString = "{" + stateName + ": ";
      int count = transitions.size();
      for(int i = 0; i < count; i ++) {
        string transitionString = transitions[i].toString();
        retString += transitionString;
        if(i != count - 1) {
          retString += ", ";
        }
      }
      return retString + "}";
    }
  };
  //String that represents the lambda (empty) move for this state machine.
  char lambda;
  //Keep a list of states that the machine has defined for it.
  State startState;
  vector<State> finalStates;
  vector<State> normalStates;
  bool startStateAdded;

public:
  NFA() {
    startStateAdded = false;
  }

  void addState(string label) {
    State s = State(label);
    normalStates.push_back(s);
  }

  void addTransition(string fromState, char symbol, string toState) {
    if(startStateAdded) {
      if(fromState == startState.getName()) {
        startState.addTransition(symbol, toState);
        return;
      }
    }

    int count = normalStates.size();
    for(int i = 0; i < count; i ++) {
      if(fromState == normalStates[i].getName()) {
        normalStates[i].addTransition(symbol, toState);
        break;
      }
    }

    count = finalStates.size();
    for(int i = 0; i < count; i ++) {
      if(fromState == finalStates[i].getName()) {
        finalStates[i].addTransition(symbol, toState);
        break;
      }
    }
  }

  void addFinalState(string label) {
    State s = State(label);
    finalStates.push_back(s);
  }

  void addStartState(string label) {
    startState = State(label);
    startStateAdded = true;
  }

  void setLambda(char lambdaLabel) {
    lambda = lambdaLabel;
  }

  string toString() {
    string retString = "{NFA: [" + startState.toString();

    int count = normalStates.size();
    if(count > 0) {
        retString += ", ";
    }

    for(int i = 0; i < count; i ++) {
      string transitionString = normalStates[i].toString();
      retString += transitionString;
      if(i != count - 1) {
        retString += ", ";
      }
    }

    count = finalStates.size();
    if(count > 0 && (startState.getName() != finalStates[0].getName())) {
        retString += ", ";
    }
    for(int i = 0; i < count; i ++) {
      if(startState.getName() != finalStates[i].getName()) {
        string transitionString = finalStates[i].toString();
        retString += transitionString;
        if(i != count - 1) {
          retString += ", ";
        }
      }
    }
    return retString + "]}";
  }
};

//Define a new structure the bundles the expression with the
//NFA
struct arg {
  NFA nfa;
  string exp;
};

void *processString(void *bundle) {
  //Keep track of where this thread is in the reading process.
  //struct arg data = *((struct arg *) bundle);
  //string exp = data.exp;
  //NFA nfa = data.nfa;
  int readPosition = 0;
  pthread_exit(NULL);
  //cout << exp;
}

bool test(NFA nfa, string expression) {
  pthread_t thread;
  cout << "here";
  struct arg bundle = {nfa, expression};
  int t = 0;
  pthread_create(&thread, NULL, processString, (void*) &t);
  cout << "here";
  pthread_exit(NULL);
  cout << "here";
  return false;
}

int main() {
  NFA nfa;
  nfa.addStartState("q1");
  nfa.addTransition("q1",'a', "q1");
  nfa.addFinalState("q1");
  string nfaString = nfa.toString();
  string testString = "kelan";
  test(nfa, testString);
  cout << nfaString;
}
