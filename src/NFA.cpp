#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <sys/time.h>
#include <iomanip>
#include <fstream>

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
  vector<arg> args;

  bool loop = true;

  while(loop) {
    vector<string> transitions = nfa.getStatesForTransition(currentState, expr[0]);
    vector<string> teleportTransitions = nfa.getStatesForTransition(currentState, lambda);

    bool emptyTransitions = teleportTransitions.size() > 0;
    bool multipleTransitions = transitions.size() > 1;

    if(emptyTransitions) {
      //create a thread for every empty transition
      for(int i = 0; i < teleportTransitions.size(); i ++) {
        pthread_t thread;
        struct arg *newBundle = new arg;
        newBundle->startState = teleportTransitions[i];
        newBundle->nfa = nfa;
        newBundle->expr = expr;
        args.push_back(*newBundle);
        pthread_create(&thread, &attr, processString, (void*) newBundle);
        threads.push_back(thread);
      }
    }

    if(multipleTransitions) {
      //create a thread for every new transition
      for(int i = 0; i < transitions.size(); i ++) {
        pthread_t thread;
        struct arg *newBundle = new arg;
        newBundle->startState = transitions[i];
        newBundle->nfa = nfa;
        newBundle->expr = expr.substr(1, expr.length() - 1);
        args.push_back(*newBundle);
        pthread_create(&thread, &attr, processString, (void*) newBundle);
        threads.push_back(thread);
      }
    }

    if(transitions.size() == 1) {
      currentState = transitions[0];
      if(expr.length() != 1) {
        expr = expr.substr(1, expr.length() - 1);
      }
      else {
        expr = "";
      }
    }
    else {
      loop = false;
    }
  }

  for(int i = 0; i < threads.size(); i ++) {
    pthread_join(threads[i], NULL);
  }

  if(nfa.isFinalState(currentState) && expr.length() == 0) {
    accepted = true;
  }

  pthread_exit(NULL);
}

bool parallelTest(const NFA &nfa, const string &expression) {
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

void serialTestFunc(string startState, NFA nfa, string expr) {
  string currentState = startState;
  char lambda = nfa.getLambda();

  bool loop = true;

  while(loop) {
    vector<string> transitions = nfa.getStatesForTransition(currentState, expr[0]);
    vector<string> teleportTransitions = nfa.getStatesForTransition(currentState, lambda);

    bool emptyTransitions = teleportTransitions.size() > 0;
    bool multipleTransitions = transitions.size() > 1;

    if(emptyTransitions) {
      for(int i = 0; i < teleportTransitions.size(); i ++) {
        serialTestFunc(teleportTransitions[i], nfa, expr);
      }
    }

    if(multipleTransitions) {
      //create a thread for every new transition
      for(int i = 0; i < transitions.size(); i ++) {
        string expression = expr.substr(1, expr.length() - 1);
        serialTestFunc(transitions[i], nfa, expression);
      }
    }

    if(transitions.size() == 1) {
      currentState = transitions[0];
      if(expr.length() != 1) {
        expr = expr.substr(1, expr.length() - 1);
      }
      else {
        expr = "";
      }
    }
    else {
      loop = false;
    }
  }

  if(nfa.isFinalState(currentState) && expr.length() == 0) {
    accepted = true;
  }
}

bool serialTest(NFA nfa, string expr) {
  accepted = false;
  serialTestFunc(nfa.getStartState(), nfa, expr);
  return accepted;
}

void printTest(NFA &nfa, string &testString, int testNum, bool parallel, bool expr) {
  cout << "String Test " << testNum << endl;
  cout << "  NFA structure to test: " << endl;
  cout << "  " << nfa.toString() << endl;
  if(expr) {
    cout << "  Testing on input string: " << testString << endl;
  }
  bool testResult;
  if(parallel) {
    testResult = parallelTest(nfa, testString);
  }
  else {
    testResult = serialTest(nfa, testString);
  }
  cout << "  String was: ";
  if(testResult) {
    cout << "ACCEPTED";
  }
  else {
    cout << "REJECTED";
  }
  cout << endl;
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

void printDashes(int n) {
  for(int i = 0; i < n; i ++) {
    cout << "-";
  }
  cout << endl;
}

long long start_timer() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

// Prints the time elapsed since the specified time
long long stop_timer(long long start_time, std::string name) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long long end_time = tv.tv_sec * 1000000 + tv.tv_usec;
  std::cout << setprecision(5);
	std::cout << name << ": " << ((float) (end_time - start_time)) / (1000 * 1000) << " sec\n";
	return end_time - start_time;
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

  int numDashes = 80;
  printDashes(numDashes);
  cout << "NFA DATA STRUCTURE TESTS" << endl;
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
  printDashes(numDashes);
  cout << "SERIAL ALGORITHM TESTS" << endl << endl;
  string testStrings[] = {"", "aaaa", "bbbb", "bba", "abaaa", "baab", "b", "bbbaabbbaa"};
  printDashes(numDashes);
  int numTests = 8;
  long long timeElapsed[numTests];
  for(int i = 0; i <numTests; i ++) {
    long long startTime = start_timer();
    printTest(nfa, testStrings[i], i, false, true);
    stop_timer(startTime, "  time taken");
    cout << endl;
  }
  printDashes(numDashes);
  cout << "PARALLEL ALGORITHM TESTS" << endl << endl;
  for(int i = 0; i < numTests; i ++) {
    long long startTime = start_timer();
    printTest(nfa, testStrings[i], i, true, true);
    stop_timer(startTime, "  time taken");
    cout << endl;
  }
  printDashes(numDashes);
}

int main(int argc, char **argv) {
  if(argc == 1) {
    testMachine();
  }
  else if(argc > 1) {
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

    if(argc == 2) {
      string pOrS = argv[1];
      ifstream read("teststrings");
      string testString;
      getline(read, testString);

      if(pOrS[0] == 'p' || pOrS[0] == 'P') {
        long long startTime = start_timer();
        printTest(nfa, testString, 0, true, false);
        stop_timer(startTime, "  time taken");
      }
      else {
        long long startTime = start_timer();
        printTest(nfa, testString, 0, false, false);
        stop_timer(startTime, "  time taken");
      }
    }
    else if(argc == 3){
      string testString = argv[1];
      string pOrS = argv[2];
      if(pOrS[0] == 'p' || pOrS[0] == 'P') {
        long long startTime = start_timer();
        printTest(nfa, testString, 0, true, true);
        stop_timer(startTime, "  time taken");
      }
      else {
        long long startTime = start_timer();
        printTest(nfa, testString, 0, false, true);
        stop_timer(startTime, "  time taken");
      }
    }
  }
}
