class optionClass {
  public:
  optionClass(){};
  optionClass(string newName){name=newName; on=true;};
  optionClass(string newName,float myValue){name=newName; on=true; value=myValue;}; 
  void enable(){on=true;}
  void disable(){on=false;};
  bool getState(){return(on);};  
  float getValue(){return(value);};
  void setName(string newName){name=newName;};
  ~optionClass(){};
 private:
  string name;  // name of the option
  bool   on;    // on/off if true false
  float  value; // value associated with the option
};

class optionSetClass {
  public:
  optionSetClass(){nOptions=0;);
  ~optionSetClass(){ nOptions=0; if(NULL != option) delete [] option;);
 private:
  int nUptions;
  optionClass *option; 
};
