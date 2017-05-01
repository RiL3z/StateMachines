#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>

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

    string getNextState() {
      return nextState;
    }

    char getInputSymbol() {
      return inputSymbol;
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

  vector<string> getStatesForTransition(string stateName, char c) {
    vector<string> states;
    //check for start state
    if(startStateAdded) {
      if(startState.getName() == stateName) {
        vector<Transition> transitions = startState.getTransitions();
        for(int i = 0; i < transitions.size(); i ++) {
          if(transitions[i].getInputSymbol() == c)
          states.push_back(transitions[i].getNextState());
        }
      }
    }
    //then for normal states
    for(int i = 0; i < normalStates.size(); i ++) {
      if(normalStates[i].getName() == stateName) {
        vector<Transition> transitions = startState.getTransitions();
        for(int i = 0; i < transitions.size(); i ++) {
          if(transitions[i].getInputSymbol() == c)
          states.push_back(transitions[i].getNextState());
        }
      }
    }

    //then for final states
    for(int i = 0; i < finalStates.size(); i ++) {
      if(startState.getName() != finalStates[i].getName()) {
        if(finalStates[i].getName() == stateName) {
          vector<Transition> transitions = startState.getTransitions();
          for(int i = 0; i < transitions.size(); i ++) {
            if(transitions[i].getInputSymbol() == c)
            states.push_back(transitions[i].getNextState());
          }
        }
      }
    }

    return states;
  }

  void addFinalState(string label) {
    State s = State(label);
    finalStates.push_back(s);
  }

  bool isFinalState(string label) {
    for(int i = 0; i < finalStates.size(); i ++) {
      if(label == finalStates[i].getName()) {
        return true;
      }
    }

    return false;
  }

  void addStartState(string label) {
    startState = State(label);
    startStateAdded = true;
  }

  string getStartState() {
    return startState.getName();
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
  string startState;
  NFA nfa;
  string exp;
};

//shared variable that all threads are working to change to
//true.
bool accepted = false;

void *processString(void *bundle) {
  //Reset for every call.
  accepted = false;
  //Keep track of where this thread is in the reading process.
  struct arg data = *((struct arg *) bundle);
  string exp = data.exp;
  NFA nfa = data.nfa;
  string currentState = nfa.getStartState();
  //Check for e moves first

  //Then check to see if the character read has any transitions
  //defined for it.
  for(int i = 0; i < exp.length(); i ++) {
    vector<string> nextStates = nfa.getStatesForTransition(currentState, exp[i]);
    if(nextStates.size() != 0) {
      if(nextStates.size() == 1) {
        currentState = nextStates[0];
      }
      else {

      }
    }
    else {
      return NULL;
    }
  }

  if(nfa.isFinalState(currentState)) {
    accepted = true;
  }

  pthread_exit(NULL);
}

bool test(NFA nfa, string expression) {
  pthread_t thread;
  struct arg bundle = {nfa.getStartState(), nfa, expression};
  int t = 0;
  pthread_create(&thread, NULL, processString, (void*) &bundle);

  pthread_join(thread, NULL);
  return accepted;
}

int main() {
  NFA nfa;
  nfa.setLambda('e');
  nfa.addStartState("q1");
  nfa.addState()
  nfa.addTransition("q1",'a', "q1");
  nfa.addFinalState("q1");
  vector<string> states = nfa.getStatesForTransition("q1", 'a');
  cout << states[0];
  string nfaString = nfa.toString();
  bool inLanguage = test(nfa, "kelan");
  bool inLanguage2 = test(nfa, "aaaab");
  cout << boolalpha;
  cout << inLanguage2;
}
