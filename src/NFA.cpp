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

    string toString() const {
      string retString = "(";
      retString += inputSymbol;
      retString += "->" + nextState + ")";
      return retString;
    }

    string getNextState() const {
      return nextState;
    }

    char getInputSymbol() const {
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

    vector<Transition> getTransitions() const {
      return transitions;
    }

    /*vector<Transition> getEmptyTransitions() {
      vector<Transition> emptyTransitions;
      for(int i = 0; i < transitions.size(); i ++) {
        if(transitions[i] == lambda) {
          emptyTransitions.push_back(transitions[i]);
        }
      }
      return emptyTransitions;
    }*/

    string getName() const {
      return stateName;
    }

    //Get a string representation of this state for display
    //purposes.
    string toString() const {
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

  vector<string> getStatesForTransition(string stateName, char c) const {
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
        vector<Transition> transitions = normalStates[i].getTransitions();
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
          vector<Transition> transitions = finalStates[i].getTransitions();
          for(int i = 0; i < transitions.size(); i ++) {
            if(transitions[i].getInputSymbol() == c) {
              states.push_back(transitions[i].getNextState());
            }
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

  bool isFinalState(string label) const {
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

  string getStartState() const {
    return startState.getName();
  }

  void setLambda(char lambdaLabel) {
    lambda = lambdaLabel;
  }

  char getLambda() const {
    return lambda;
  }

  string toString() const {
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
  string expr;
};

//shared variable that all threads are working to change to
//true.
bool accepted = false;

void *processString(void *bundle) {
  //Keep track of where this thread is in the reading process.
  struct arg data = *((struct arg *) bundle);
  string expr = data.expr;
  string startState = data.startState;
  NFA nfa = data.nfa;
  string currentState = startState;
  cout << currentState;
  //Check for e moves first
  char lambda = nfa.getLambda();
  vector<string> teleportStates = nfa.getStatesForTransition(currentState, lambda);
  //Create an array of pthreads for every state that we teleport to.
  pthread_t threads[teleportStates.size()];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  struct arg newArgs[teleportStates.size()];
  
  for(int i = 0; i < teleportStates.size(); i ++) {
    struct arg newBundle;
    newBundle.startState = teleportStates[i];
    newBundle.nfa = nfa;
    newBundle.expr = expr;
    newArgs[i] = newBundle; 
    pthread_create(&threads[i], &attr, processString, (void*) &newArgs[i]);
    //pthread_join(threads[i], NULL);
  }

  //Then check to see if the character read has any transitions
  //defined for it.
  for(int i = 0; i < expr.length(); i ++) {
    vector<string> nextStates = nfa.getStatesForTransition(currentState, expr[i]);
    if(nextStates.size() != 0) {
      if(nextStates.size() == 1) {
        currentState = nextStates[0];
      }
      else {
        //more parallel processing here
      }
    }
    pthread_exit(NULL);
  }

  if(nfa.isFinalState(currentState)) {
    accepted = true;
  }

  for(int i = 0; i < teleportStates.size(); i ++) {
    pthread_join(threads[i], NULL);
  }

  pthread_exit(NULL);
}

bool test(const NFA &nfa, const string &expression) {
  accepted = false;
  pthread_t thread;
  struct arg bundle = {nfa.getStartState(), nfa, expression};
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&thread, &attr, processString, (void*) &bundle);
  pthread_join(thread, NULL);
  return accepted;
}

int main() {
  NFA nfa;
  nfa.setLambda('e');
  nfa.addStartState("q0");
  nfa.addState("q1");
  nfa.addState("q2");
  nfa.addFinalState("q3");
  nfa.addTransition("q0", 'e', "q1");
  nfa.addTransition("q0",'e', "q2");
  nfa.addTransition("q0", 'e', "q3");
  nfa.addTransition("q3", 'a', "q3");
  //cout << nfa.isFinalState("q3");
  vector<string> states = nfa.getStatesForTransition("q3", 'a');
  //cout << states.size();
  string nfaString = nfa.toString();
  string testString = "aaa";
  bool inLanguage = test(nfa, testString);
  //bool inLanguage2 = test(nfa, "aaaab");
  cout << inLanguage;
  //cout << inLanguage2;
  //pthread_exit(NULL);
}
