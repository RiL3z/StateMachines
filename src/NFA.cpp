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

private:
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
    lambda = 'e';
  }
  
  bool hasEmptyTransitions(string state) const{
    vector<string> transitions = getStatesForTransition(state, lambda); 
    return transitions.size() > 0 ? true: false;
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
  char lambda = nfa.getLambda();

  //All threads are joinable
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  
  //keep track of all threads that were created... 
  vector<pthread_t> threads;
  //Keep all of the different thread arguments in memory.
  vector<arg> args;
 
  while(expr != "") {
    vector<string> transitions = nfa.getStatesForTransition(currentState, lambda); 
    vector<string> teleportTransitions = nfa.getStatesForTransition(currentState, lambda);

    bool emptyTransitions = teleportTransitions.size() > 0;

    if(emptyTransitions) {
      //create a thread for every empty transition  
      for(int i = 0; i < teleportTransitions.size(); i ++) {
        struct arg newBundle;
        pthread_t thread;
        pthread_create(&thread, &attr, processString, (void*) 
      }  
    }
  } 
  /* 
  //cout << currentState;
  //Check for e moves first
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
  int j = 0;
  struct arg newArgs2[nfa.getStatesForTransition(currentState, expr[j]).size()];
  pthread_t threads2[nfa.getStatesForTransition(currentState, expr[j]).size()];
  for(; j < expr.length(); j ++) {
    vector<string> nextStates = nfa.getStatesForTransition(currentState, expr[j]);
    if(nextStates.size() != 0) {
      if(nextStates.size() == 1) {
        currentState = nextStates[0];
      }
      else {
        //more parallel processing here
        for(int k = 0; k < nextStates.size(); k ++) {
          if(currentState != nextStates[k]) {
            //cout << nextStates[k];
            struct arg newBundle;
            newBundle.startState = nextStates[k];
            newBundle.nfa = nfa; 
            newBundle.expr = expr.substr(j+1, expr.length() - 1);
            newArgs2[k] = newBundle;
            pthread_create(&threads2[k], &attr, processString, (void*) &newArgs2[k]);
          }
        }
        break;
      }
    }
    else {
      pthread_exit(NULL);
    }
    j++;
  }

  if(nfa.isFinalState(currentState)) {
    accepted = true;
  }

  for(int i = 0; i < nfa.getStatesForTransition(currentState, expr[j]).size(); i ++) {
    pthread_join(threads2[i], NULL);
  }
  
  for(int i = 0; i < teleportStates.size(); i ++) {
    pthread_join(threads[i], NULL);
  }*/
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

void printTest(NFA &nfa, string &testString, int testNum) {
  cout << "String Test " << testNum << endl;
  cout << "  NFA structure to test: " << endl;
  cout << "  " << nfa.toString() << endl;
  cout << "  Testing on input string: " << testString << endl;
  bool testResult = test(nfa, testString);
  cout << "  String was: ";
  if(testResult) {
    cout << "accepted";
  } 
  else {
    cout << "rejected";
  }
  cout << endl;
}

// The simplest test is for a state machine that accepts the empty string
void testStartIsFinal() {
  NFA nfa;
  nfa.addStartState("q0");
  nfa.addFinalState("q0"); 
  string testString = "";
  printTest(nfa, testString, 0); 
  testString = "a";
  printTest(nfa, testString, 1); 
}

string getVectorAsString(vector<string> &vect) {
  string result = "[";
  int size = vect.size();

  for(int i = 0; i < size; i ++) {
    if(i != size - 1) { 
      result += vect[i] + ", ";
    }
    else {
      result += vect[i];
    }
  }
  return result + "]";
}

bool vectorsSame(vector<string> v1, vector<string> v2) {
  if(v1.size() != v2.size()) {
    return false;
  }
  
  for(int i = 0; i < v1.size(); i ++) {
    if(v1[i] != v2[i]) {
      return false;
    }
  }
  return true;
}

void testTransition(NFA &nfa, string state, char input, int testNum, vector<string> nextStates) {
  cout << "Transition Test " << testNum << endl; 
  cout << "  State input combination: (" << state << ", " << input  << ")" << endl;
  vector<string> states = nfa.getStatesForTransition(state, input);
  string statesString = getVectorAsString(states);
  cout << "  Actual resultant states from transition: " << statesString << endl;
  if(vectorsSame(states, nextStates)) {
    cout << "  TEST PASSED" << endl; 
  }
  else {
    cout << "  TEST FAILED" << endl;
  } 
}

void testMachine() {
  NFA nfa;
  nfa.setLambda('e');
  nfa.addStartState("q0");
  nfa.addFinalState("q1");
  nfa.addState("q2");
  nfa.addFinalState("q3");
  nfa.addFinalState("q4");
  nfa.addState("q5");
  nfa.addTransition("q0", 'e', "q1");
  nfa.addTransition("q0", 'e', "q2");
  nfa.addTransition("q1", 'a', "q1");
  nfa.addTransition("q2", 'b', "q3");
  nfa.addTransition("q2", 'b', "q4");
  nfa.addTransition("q3", 'b', "q3");
  nfa.addTransition("q4", 'b', "q4");
  nfa.addTransition("q4", 'a', "q5");
  nfa.addTransition("q5", 'b', "q5");
  nfa.addTransition("q5", 'a', "q4");

  cout << "NFA structure to test: " << endl;
  cout << nfa.toString() << endl << endl;
  vector<string> nextStates1;
  nextStates1.push_back("q1");
  nextStates1.push_back("q2");
  testTransition(nfa, "q0", 'e', 0, nextStates1);  
  vector<string> nextStates2;
  nextStates2.push_back("q1");
  testTransition(nfa, "q1", 'a', 1, nextStates2);
  vector<string> nextStates3;
  nextStates3.push_back("q3");
  nextStates3.push_back("q4");
  testTransition(nfa, "q2", 'b', 2, nextStates3);
  vector<string> nextStates4;
  nextStates4.push_back("q3");
  testTransition(nfa, "q3", 'b', 3, nextStates4);
  vector<string> nextStates5;
  nextStates5.push_back("q4");
  testTransition(nfa, "q4", 'b', 4, nextStates5);
  vector<string> nextStates6;
  nextStates6.push_back("q5");
  testTransition(nfa, "q4", 'a', 5, nextStates6);
  vector<string> nextStates7;
  nextStates7.push_back("q5");
  testTransition(nfa, "q5", 'b', 6, nextStates7);
  vector<string> nextStates8;
  nextStates8.push_back("q4");
  testTransition(nfa, "q5", 'a', 7, nextStates8);

  cout << endl;
 
  string test1 = "aaaa"; 
  printTest(nfa, test1, 0); 
  
  string test2 = "";
  printTest(nfa, test2, 1);
} 

void testNoEmptyTransitions() {
  NFA nfa;
  nfa.addStartState("q0");
  nfa.addFinalState("q0");
  

}

/*void test1() {
  NFA nfa;
  nfa.setLambda('e');
  nfa.addStartState("q0");
  nfa.addState("q1");
  nfa.addState("q2");
  nfa.addFinalState("q3");
  nfa.addFinalState("q4");
  nfa.addFinalState("q5"); 
  nfa.addTransition("q0", 'a', "q1");
  nfa.addTransition("q0", 'a', "q2");
  nfa.addTransition("q2", 'c', "q2");
  nfa.addTransition("q2", 'e', "q3");
  nfa.addTransition("q3", 'b', "q3");
  nfa.addTransition("q1", 'e', "q4");
  nfa.addTransition("q1", 'e', "q5");
  nfa.addTransition("q4", 'a', "q4");
  nfa.addTransition("q5", 'b', "q5"); 
  cout << "NFA structure to test: " << endl;
  cout << nfa.toString() << endl;
}*/

int main(void) {
 testMachine(); 
}
