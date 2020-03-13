#include <iostream>
#include <vector>
#include <queue>
#include <sstream>
#include <map>
#include <fstream>
using namespace std;

enum InstructionType
{
    NOP = 0, // NOP. Pipeline bubble.
    ADD,     // Add
    SUB,     // Subtract
    MULT,    // Multiply
    DIV,     // Divide
    LW,      // Load word
    SW,      // Store word
    BNE      // Branch not equal
};

enum ReservationStationType
{
    ADDDER = 0,
    MULTDIV,
    MEMORY,
    BRANCH
};

string ReservationStationTypeString[] = {"ADD", "MULT", "MEM", "BRANCH"};
enum Stage
{
    NOTISSUED = -1,
    ISSUE = 0,
    EXEC,
    MEM,
    WRITEBACK,
    COMMIT,
    FINISHED
};

class Instruction
{
public:
    Instruction();
    Instruction(string, int);
    int getId()
    {
        return id;
    }
    Stage getStage()
    {
        return stage;
    }

    InstructionType type; // Type of instruction
    int dest;             // Destination register number
    int src1;             // Source register number
    int src2;             // Source register number
    void printInstruction();
    int issue;
    int execStart;
    int execEnd;
    int memoryStart;
    int memoryEnd;
    int writeCDB;
    int commit;
    int id;
    Stage stage;
};

Instruction::Instruction(void)
{

    type = NOP;
    dest = -1;
    src1 = -1;
    src2 = -1;
    id = -1;
    issue = -1;
    execStart = -1;
    execEnd = -1;
    memoryStart = -1;
    memoryEnd = -1;
    writeCDB = -1;
    commit = -1;
    stage = NOTISSUED;
}

Instruction::Instruction(string newInst, int id)
{
    this->id = id;
    string buf;
    stringstream ss(newInst);
    vector<string> tokens;

    while (ss >> buf)
    {
        tokens.push_back(buf);
    }

    if (tokens[0] == "ADD")
        type = ADD;
    else if (tokens[0] == "SUB")
        type = SUB;
    else if (tokens[0] == "MULT")
        type = MULT;
    else if (tokens[0] == "DIV")
        type = DIV;
    else if (tokens[0] == "LW")
        type = LW;
    else if (tokens[0] == "SW")
        type = SW;
    else if (tokens[0] == "BNE")
        type = BNE;
    else
        type = NOP;

    dest = -1;
    src1 = -1;
    src2 = -1;

    if (tokens.size() > 1)
    {
        dest = atoi(tokens[1].erase(0, 1).c_str());
    }
    if (tokens.size() > 2)
    {
        src1 = atoi(tokens[2].erase(0, 1).c_str());
    }
    if (tokens.size() > 3)
    {
        src2 = atoi(tokens[3].erase(0, 1).c_str());
    }

    // Store and BNE has 2 source operands and no destination operand
    if (type == SW || type == BNE)
    {
        src2 = src1;
        src1 = dest;
        dest = -1;
    }

    issue = -1;
    execStart = -1;
    execEnd = -1;
    memoryStart = -1;
    memoryEnd = -1;
    writeCDB = -1;
    commit = -1;
    stage = NOTISSUED;
}

class Register
{
public:
    Register();
    void clearDependencyValues(string dependentValue);
    string dataValue; // takes reservation station number.
    int registerNumber;
    std::string registerName;
};

Register::Register(void)
{

    dataValue = "";
    registerNumber = -1;
    registerName = "";
}

void Register::clearDependencyValues(string dependentValue)
{
    if (dataValue == dependentValue)
    {
        dataValue = "";
    }
}

Register rf[15];

class ReservationStation
{
    ReservationStationType type;
    bool busy;
    string op;
    string vj;
    string vk;
    string qj;
    string qk;
    Instruction *instruction;
    int cycleToComplete;

public:
    ReservationStation()
    {
        busy = false;
        op = "";
        vj = "";
        vk = "";
        qj = "";
        qk = "";
        instruction = NULL;
        cycleToComplete = -1;
    }

    ReservationStation(Instruction *instr)
    {
        instruction = instr;
    }

    bool isInWriteBackStage()
    {
        return instruction->getStage() == WRITEBACK;
    }

    bool isInMemoryStage()
    {
        return instruction->getStage() == MEM;
    }

    bool isInExecStage()
    {
        return instruction->getStage() == EXEC;
    }

    bool isInIssueStage()
    {
        return instruction->getStage() == ISSUE;
    }

    int getIssueEndTime()
    {
        return instruction->issue;
    }

    void setStage(Stage stage)
    {
        instruction->stage = stage;
    }

    int getExecOrMemEndTime()
    {
        if ((instruction->type == LW || instruction->type == SW) && instruction->memoryEnd != -1)
        {
            return instruction->memoryEnd;
        }
        // else if (instruction->type == SW && instruction->issue != -1)
        // {
        //     return instruction->issue;
        // }
        return instruction->execEnd;
    }

    int getWriteBackEndTime()
    {
        return instruction->writeCDB;
    }

    ReservationStationType getType()
    {
        return type;
    }
    void setType(ReservationStationType type)
    {
        this->type = type;
    }
    InstructionType getInstructionType()
    {
        return instruction->type;
    }

    int getId()
    {
        return instruction->getId();
    }

    bool hasDependency()
    {
        if (qj != "" || qk != "")
            return true;
        return false;
    }
    void setqj(string qj)
    {
        this->qj = qj;
    }
    void setqk(string qk)
    {
        this->qk = qk;
    }
    void clearDependency(string reservationDependency)
    {
        if (qj == reservationDependency)
        {
            qj = "";
        }
        else if (qk == reservationDependency)
        {
            qk = "";
        }
    }

    void setMemoryTiming(int startTime, int endTime)
    {
        instruction->memoryStart = startTime;
        instruction->memoryEnd = endTime;
    }

    void setExecTiming(int startTime, int endTime)
    {
        instruction->execStart = startTime;
        instruction->execEnd = endTime;
    }

    void setWriteBackTiming(int time)
    {
        instruction->writeCDB = time;
    }

    Instruction *getInstruction()
    {
        return instruction;
    }
};

struct compare
{
    bool operator()(Instruction *a, Instruction *b)
    {
        return a->getId() > b->getId();
    }
};

class ReservationStationTable
{
protected:
    vector<ReservationStation *> add, mult, branch, memory;
    int addUnits, multUnits, branchUnits, memoryUnits;
    bool isMemBusy, isMemExecBusy, isAdderBusy, isMultBusy, isBranchBusy;
    int memoryCycle, adderCycle, multCycle, branchCycle;
    deque<int> branchInstrStallQueue;
    bool isBranchTaken, isBranchEncountered;
    bool isSpeculative, speculativeShouldExitOnBranch;
    int ROBHead, branchIdExit;
    priority_queue<Instruction *, vector<Instruction *>, compare> ROB;

public:
    ReservationStationTable()
    {
        isMemBusy = isMemExecBusy = isAdderBusy = isMultBusy = isBranchBusy = false;
        memoryCycle = adderCycle = multCycle = branchCycle = -1;
        ROBHead = 0;
        speculativeShouldExitOnBranch = false;
        branchIdExit = -1;
    }

    ReservationStationTable(int addUnits, int multUnits, int branchUnits, int memoryUnits, int memoryCycle, int adderCycle, int multCycle, int branchCycle, bool isBranchTaken, bool isSpeculative)
    {
        this->addUnits = addUnits;
        this->multUnits = multUnits;
        this->branchUnits = branchUnits;
        this->memoryUnits = memoryUnits;
        this->memoryCycle = memoryCycle;
        this->adderCycle = adderCycle;
        this->multCycle = multCycle;
        this->branchCycle = branchCycle;
        isMemBusy = isMemExecBusy = isAdderBusy = isMultBusy = isBranchBusy = false;
        this->isBranchTaken = isBranchTaken;
        isBranchEncountered = false;
        this->isSpeculative = isSpeculative;
        ROBHead = 0;
        speculativeShouldExitOnBranch = false;
        branchIdExit = -1;
    }

    bool isEmpty()
    {
        return add.empty() && mult.empty() && branch.empty() && memory.empty() && ROB.empty();
    }

    void commit(int cycleTime, int topK)
    {
        for (int i = topK; i > 0; i--)
        {
            if (!ROB.empty() && ROB.top()->getId() != ROBHead)
            {
                //in order commit
                break;
            }

            //else you can commit
            if (!ROB.empty())
            {
                ROB.top()->stage = FINISHED;
                ROB.top()->commit = cycleTime;
                if (ROB.top()->type == SW)
                {
                    ROB.top()->memoryStart = -1;
                    ROB.top()->memoryEnd = -1;
                }
                ROB.pop();
                ROBHead++;
            }
        }
    }

    void addAvailableWriteBackToMap(vector<ReservationStation *> iterateReservation, map<int, pair<ReservationStationType, int>> &availableToWriteBack, int cycleTime)
    {
        for (int i = 0; i < iterateReservation.size(); i++)
        {

            if (iterateReservation[i]->isInWriteBackStage() && iterateReservation[i]->getExecOrMemEndTime() != -1 && iterateReservation[i]->getExecOrMemEndTime() < cycleTime)
            {
                pair<ReservationStationType, int> p;
                p = make_pair(iterateReservation[i]->getType(), i + 1);
                availableToWriteBack[iterateReservation[i]->getId()] = p;
            }
        }
    }

    void writeBack(int cycleTime, int topK)
    {
        map<int, pair<ReservationStationType, int>> availableToWriteBack;
        // iterate over add reservation station
        addAvailableWriteBackToMap(add, availableToWriteBack, cycleTime);
        // iterate over mult reservation station
        addAvailableWriteBackToMap(mult, availableToWriteBack, cycleTime);
        // iterate over load reservation station
        addAvailableWriteBackToMap(memory, availableToWriteBack, cycleTime);

        // clear write backs
        auto it = availableToWriteBack.begin();
        for (int i = topK; i > 0 && it != availableToWriteBack.end(); i--, it++)
        {
            // make it finished
            //EX : ADD1
            string clearDependency = ReservationStationTypeString[it->second.first];

            //clear entry in table.
            if (it->second.first == ADDDER)
            {
                //isAdderBusy = false;
                clearDependency += to_string(add[it->second.second - 1]->getInstruction()->id);
                add[it->second.second - 1]
                    ->setWriteBackTiming(cycleTime);
                if (!isSpeculative)
                {
                    add[it->second.second - 1]
                        ->setStage(FINISHED);
                    add.erase(add.begin() + it->second.second - 1);
                }
            }
            else if (it->second.first == MULTDIV)
            {
                //isMultBusy = false;
                clearDependency += to_string(mult[it->second.second - 1]->getInstruction()->id);
                mult[it->second.second - 1]->setWriteBackTiming(cycleTime);
                if (!isSpeculative)
                {
                    mult[it->second.second - 1]->setStage(FINISHED);
                    mult.erase(mult.begin() + it->second.second - 1);
                }
            }
            else if (it->second.first == MEMORY)
            {
                //isMemBusy = false;
                clearDependency += to_string(memory[it->second.second - 1]->getInstruction()->id);
                memory[it->second.second - 1]->setWriteBackTiming(cycleTime);
                if (!isSpeculative)
                {
                    memory[it->second.second - 1]->setStage(FINISHED);
                    memory.erase(memory.begin() + (it->second.second - 1));
                }
            }

            clearWriteBack(clearDependency);
        }
    }

    void clearWriteBack(string reservationNumber)
    {
        clearReservationTableDependency(reservationNumber, add);
        clearReservationTableDependency(reservationNumber, mult);
        clearReservationTableDependency(reservationNumber, memory);
        clearReservationTableDependency(reservationNumber, branch);
        clearRegister(reservationNumber);
    }

    void clearReservationTableDependency(string reservationNumber, vector<ReservationStation *> reservationStation)
    {
        for (auto reserv : reservationStation)
        {
            reserv->clearDependency(reservationNumber);
        }
    }

    void clearRegister(string reservationNumber)
    {
        for (int i = 0; i < 10; i++)
        {
            rf[i].clearDependencyValues(reservationNumber);
        }
    }

    void execMemory(int cycleTime)
    {
        vector<ReservationStation *>::iterator it = memory.begin();
        while (it != memory.end())
        {
            int timing;
            if ((*it)->getInstructionType() == LW)
            {
                timing = (*it)->getExecOrMemEndTime();
            }
            else
            {
                timing = (*it)->getIssueEndTime();
            }

            if (!isMemBusy && (*it)->isInMemoryStage() && !(*it)->hasDependency() && timing != -1 && timing < cycleTime)
            {
                isMemBusy = true;
                (*it)->setMemoryTiming(cycleTime, cycleTime + memoryCycle - 1);

                if ((*it)->getInstructionType() == SW && !isSpeculative)
                {

                    // if non-speculative
                    (*it)->setStage(FINISHED);
                    memory.erase(it);
                    isMemBusy = false;
                }
                else if (isSpeculative && (*it)->getInstructionType() == SW)
                {
                    (*it)->setStage(COMMIT);
                    isMemBusy = false;
                    ROB.push((*it)->getInstruction());
                    memory.erase(it);
                }
                break;
            }

            it++;
        }
    }

    void execute(int cycleTime)
    {
        // 1. LOAD
        for (auto it : memory)
        {
            if (!isMemExecBusy && it->isInExecStage() && !it->hasDependency() && it->getIssueEndTime() != -1 && it->getIssueEndTime() < cycleTime)
            {
                isMemExecBusy = true;
                it->setExecTiming(cycleTime, cycleTime);
                break;
            }
        }

        //2. ADD
        for (auto it : add)
        {
            if (!isAdderBusy && it->isInExecStage() && !it->hasDependency() && it->getIssueEndTime() != -1 && it->getIssueEndTime() < cycleTime)
            {
                isAdderBusy = true;
                it->setExecTiming(cycleTime, cycleTime + adderCycle - 1);
                break;
            }
        }

        //3. MULT
        for (auto it : mult)
        {
            if (!isMultBusy && it->isInExecStage() && !it->hasDependency() && it->getIssueEndTime() != -1 && it->getIssueEndTime() < cycleTime)
            {
                isMultBusy = true;
                it->setExecTiming(cycleTime, cycleTime + multCycle - 1);
                break;
            }
        }

        //4. Branch
        for (auto it : branch)
        {
            if (!isBranchBusy && it->isInExecStage() && !it->hasDependency() && it->getIssueEndTime() != -1 && it->getIssueEndTime() < cycleTime)
            {
                isBranchBusy = true;
                it->setExecTiming(cycleTime, cycleTime + branchCycle - 1);
                break;
            }
        }
    }

    bool canIssue(InstructionType instructionType)
    {
        if ((!isSpeculative) && ((isBranchEncountered) && (!isBranchTaken)))
        {
            // if branch is not taken, don't issue any instruction after branch
            return false;
        }

        if (isSpeculative && speculativeShouldExitOnBranch)
        {
            return false;
        }

        if (instructionType == LW || instructionType == SW)
        {
            return memory.size() < memoryUnits;
        }
        else if (instructionType == ADD || instructionType == SUB)
        {
            return add.size() < addUnits;
        }
        else if (instructionType == MULT || instructionType == DIV)
        {
            return mult.size() < memoryUnits;
        }
        else if (instructionType == BNE)
        {
            isBranchEncountered = true;
            return branch.size() < branchUnits;
        }
    }

    void issue(Instruction *instr, int cycleTime)
    {
        instr->issue = cycleTime;
        instr->stage = ISSUE;
        if (instr->type == LW || instr->type == SW)
        {
            ReservationStation *mem = new ReservationStation(instr);
            mem->setType(MEMORY);
            memory.push_back(mem);
            if (instr->src1 != -1 && rf[instr->src1].dataValue != "")
            {
                mem->setqj(rf[instr->src1].dataValue);
            }
            if (instr->src2 != -1 && rf[instr->src2].dataValue != "")
            {
                mem->setqk(rf[instr->src2].dataValue);
            }

            if (instr->dest != -1)
            {
                rf[instr->dest].dataValue = ReservationStationTypeString[MEMORY] + to_string(instr->id);
            }
        }
        else if (instr->type == ADD || instr->type == SUB)
        {
            ReservationStation *adder = new ReservationStation(instr);
            adder->setType(ADDDER);
            add.push_back(adder);
            if (instr->src1 != -1 && rf[instr->src1].dataValue != "")
            {
                adder->setqj(rf[instr->src1].dataValue);
            }
            if (instr->src2 != -1 && rf[instr->src2].dataValue != "")
            {
                adder->setqk(rf[instr->src2].dataValue);
            }

            if (instr->dest != -1)
            {
                rf[instr->dest].dataValue = ReservationStationTypeString[ADDDER] + to_string(instr->id);
            }
        }
        else if (instr->type == MULT || instr->type == DIV)
        {
            ReservationStation *multDiv = new ReservationStation(instr);
            multDiv->setType(MULTDIV);
            mult.push_back(multDiv);
            if (instr->src1 != -1 && rf[instr->src1].dataValue != "")
            {
                multDiv->setqj(rf[instr->src1].dataValue);
            }
            if (instr->src2 != -1 && rf[instr->src2].dataValue != "")
            {
                multDiv->setqk(rf[instr->src2].dataValue);
            }

            if (instr->dest != -1)
            {
                rf[instr->dest].dataValue = ReservationStationTypeString[MULTDIV] + to_string(instr->id);
            }
        }
        else if (instr->type == BNE)
        {
            ReservationStation *br = new ReservationStation(instr);
            br->setType(BRANCH);
            branch.push_back(br);
            if (instr->src1 != -1 && rf[instr->src1].dataValue != "")
            {
                br->setqj(rf[instr->src1].dataValue);
            }
            if (instr->src2 != -1 && rf[instr->src2].dataValue != "")
            {
                br->setqk(rf[instr->src2].dataValue);
            }

            if (!isSpeculative)
            {
                branchInstrStallQueue.push_back(instr->issue);
            }
        }
    }

    void advanceStage(int cycleTime)
    {

        //6. Writeback to commit
        advanceFromWriteBack(cycleTime, add);
        advanceFromWriteBack(cycleTime, mult);
        advanceFromWriteBack(cycleTime, memory);

        //5. Branch case
        vector<ReservationStation *>::iterator it = branch.begin();
        while (it != branch.end())
        {
            if ((*it)->isInExecStage() && (*it)->getExecOrMemEndTime() != -1 && (*it)->getExecOrMemEndTime() <= cycleTime && isBranchBusy)
            {
                isBranchBusy = false;
                if (!isSpeculative)
                {
                    (*it)->setStage(FINISHED);

                    // remove stall
                    branchInstrStallQueue.pop_front();
                }

                if (isSpeculative && isBranchTaken)
                {
                    (*it)->setStage(COMMIT);
                    ROB.push((*it)->getInstruction());
                }
                else if (isSpeculative && !isBranchTaken)
                {
                    (*it)->setStage(COMMIT);
                    ROB.push((*it)->getInstruction());
                    branchIdExit = (*it)->getInstruction()->getId();
                    speculativeShouldExitOnBranch = true;
                }

                //remove branch instr from queue
                branch.erase(it);
                break;
            }
            it++;
        }

        //4. Exec to WriteBack
        advanceFromExecStage(cycleTime, add);
        advanceFromExecStage(cycleTime, mult);

        // 3. Memory to WriteBack
        for (auto it : memory)
        {
            if ((it)->isInMemoryStage() && (it)->getExecOrMemEndTime() != -1 && (it)->getExecOrMemEndTime() <= cycleTime)
            {

                (it)->setStage(WRITEBACK);
                isMemBusy = false;
            }

            it++;
        }

        // 2. Exec to memory
        for (auto it : memory)
        {
            if (it->getInstructionType() != SW && it->isInExecStage() && it->getExecOrMemEndTime() != -1 && it->getExecOrMemEndTime() <= cycleTime)
            {
                it->setStage(MEM);
                isMemExecBusy = false;
            }
        }

        // 1. Issue to Exec/ MEM
        advanceFromIssueStage(cycleTime, memory);
        advanceFromIssueStage(cycleTime, add);
        advanceFromIssueStage(cycleTime, mult);
        advanceFromIssueStage(cycleTime, branch);
    }

    void advanceFromIssueStage(int cycleTime, vector<ReservationStation *> resrv)
    {
        for (auto it : resrv)
        {
            if (((!branchInstrStallQueue.empty() && it->getIssueEndTime() <= branchInstrStallQueue.front()) || (branchInstrStallQueue.empty())) && it->getInstructionType() != SW && it->isInIssueStage() && it->getIssueEndTime() <= cycleTime)
            {
                it->setStage(EXEC);
            }

            if (((!branchInstrStallQueue.empty() && it->getIssueEndTime() <= branchInstrStallQueue.front()) || (branchInstrStallQueue.empty())) && it->getInstructionType() == SW && it->isInIssueStage() && it->getIssueEndTime() <= cycleTime)
            {
                it->setStage(MEM);
            }
        }
    }

    void advanceFromExecStage(int cycleTime, vector<ReservationStation *> resrv)
    {
        for (auto it : resrv)
        {
            if (it->isInExecStage() && it->getExecOrMemEndTime() != -1 && it->getExecOrMemEndTime() <= cycleTime)
            {
                it->setStage(WRITEBACK);
                if (it->getInstructionType() == ADD || it->getInstructionType() == SUB)
                    isAdderBusy = false;
                if (it->getInstructionType() == MULT || it->getInstructionType() == DIV)
                    isMultBusy = false;
            }
        }
    }

    void advanceFromWriteBack(int cycleTime, vector<ReservationStation *> &resrv)
    {
        vector<ReservationStation *>::iterator it = resrv.begin();

        while (it != resrv.end())
        {
            if ((*it)->isInWriteBackStage() && (*it)->getWriteBackEndTime() != -1 && (*it)->getWriteBackEndTime() <= cycleTime)
            {
                (*it)->setStage(COMMIT);
                ROB.push((*it)->getInstruction());
                resrv.erase(it);
            }
            else
                it++;
        }
    }

    void speculativeReservationClear()
    {
        if (!speculativeShouldExitOnBranch || branchIdExit == -1)
            return;

        speculativeClear(add);
        speculativeClear(mult);
        speculativeClear(branch);
        speculativeClear(memory);

        while (!ROB.empty() && ROB.top()->getId() > branchIdExit)
        {
            ROB.top()->stage = NOTISSUED;
            ROB.pop();
        }
    }

    void speculativeClear(vector<ReservationStation *> &resrv)
    {
        vector<ReservationStation *>::iterator it = resrv.begin();

        while (it != resrv.end())
        {
            if ((*it)->getId() > branchIdExit)
            {
                (*it)->setStage(NOTISSUED);
                resrv.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
};

class TomsuloSimulator
{
    vector<Instruction *> *instructions;
    ReservationStationTable *reservationTable;
    int issueCount, commitCount;

public:
    TomsuloSimulator()
    {
        reservationTable = NULL;
    }

    TomsuloSimulator(vector<Instruction *> *instr, int addUnits, int multUnits, int branchUnits, int memoryUnits, int memoryCycle, int adderCycle, int multCycle, int branchCycle, int issueCount, int commitCount, bool isBranchTaken, bool isSpeculative)
    {
        instructions = instr;
        reservationTable = new ReservationStationTable(addUnits, multUnits, branchUnits, memoryUnits, memoryCycle, adderCycle, multCycle, branchCycle, isBranchTaken, isSpeculative);
        this->issueCount = issueCount;
        this->commitCount = commitCount;
    }

    void printTimingCycle()
    {
        cout << "Issue"
             << "\t"
             << "Exec"
             << "\t"
             << "Mem"
             << "\t"
             << "WrtCDB"
             << "\t"
             << "Commit" << endl;
        for (auto it : (*instructions))
        {
            if (it->getStage() != FINISHED)
                continue;

            if (it->issue == -1)
                continue;

            cout << (it)->issue << "\t";

            if (it->execStart != -1 && it->execEnd != -1)
            {
                cout << it->execStart << "-" << it->execEnd << "\t";
            }
            else
            {
                cout << "\t";
            }

            if (it->memoryStart != -1 && it->memoryEnd != -1)
            {
                cout << it->memoryStart << "-" << it->memoryEnd << "\t";
            }
            else
            {
                cout << "\t";
            }
            if (it->writeCDB != -1)
                cout << it->writeCDB << "\t";
            else
            {
                cout << "\t";
            }
            if (it->commit != -1)
            {
                cout << it->commit;
            }

            cout << endl;
        }
    }
    void execute();
};

void TomsuloSimulator::execute()
{
    int time = 1;
    auto it = (*instructions).begin();
    do
    {
        int i = 0;
        while ((i < issueCount) && (it != (*instructions).end()))
        {
            bool canIssue = reservationTable->canIssue((*it)->type);
            if (canIssue)
            {
                reservationTable->issue(*it, time);
                if ((*it)->type == BNE)
                {
                    it++;
                    break;
                }
                it++;
            }

            i++;
        }
        reservationTable->execute(time);
        reservationTable->execMemory(time);
        reservationTable->writeBack(time, commitCount);
        reservationTable->commit(time, commitCount);
        reservationTable->advanceStage(time);
        reservationTable->speculativeReservationClear();
        time++;
    } while (!reservationTable->isEmpty());
}

vector<Instruction *> *readFile(string fileName)
{
    string sLine = "";
    Instruction *newInstruction;
    ifstream infile;

    vector<Instruction *> *instructions = new vector<Instruction *>();

    infile.open(fileName.c_str(), std::ifstream::in);

    if (!infile)
    {
        std::cout << "Failed to open file " << fileName << std::endl;
        return instructions;
    }
    int instructionCount = 0;
    while (!infile.eof())
    {
        getline(infile, sLine);
        if (sLine.empty())
            break;
        newInstruction = new Instruction(sLine, instructionCount);
        instructions->push_back(newInstruction);
        instructionCount++;
    }

    infile.close();
    cout << "Read file completed!!" << endl;

    return instructions;
}

int main(int argc, char *argv[])
{
    vector<Instruction *> *instrArray;

    // TC1
    // Instruction *instr = new Instruction("LW r6 r2", 0);
    // instrArray.push_back(instr);
    // instrArray.push_back(new Instruction("LW r2 r3", 1));
    // instrArray.push_back(new Instruction("MULT r0 r2 r4", 2));
    // instrArray.push_back(new Instruction("SUB r8 r6 r2", 3));
    // instrArray.push_back(new Instruction("DIV r9 r0 r6", 4));
    // instrArray.push_back(new Instruction("ADD r6 r8 r2", 5));

    //TC2
    // instrArray.push_back(new Instruction("LW r2 r1", 0));
    // instrArray.push_back(new Instruction("ADD r2 r2 r8", 1));
    // instrArray.push_back(new Instruction("SW r2 r1", 2));
    // instrArray.push_back(new Instruction("ADD r1 r1 r9", 3));
    // instrArray.push_back(new Instruction("BNE r2 r3", 4));
    // instrArray.push_back(new Instruction("LW r2 r1", 5));
    // instrArray.push_back(new Instruction("ADD r2 r2 r8", 6));
    // instrArray.push_back(new Instruction("SW r2 r1", 7));
    // instrArray.push_back(new Instruction("ADD r1 r1 r9", 8));
    // instrArray.push_back(new Instruction("BNE r2 r3", 9));

    //TomsuloSimulator tm(instrArray, 3, 3, 2, 3, 1, 2, 10, 1, 2, 2);

    if (argc <= 1)
    {
        return -1;
    }
    string trueString("true");
    string fileName(argv[1]);
    instrArray = readFile(fileName);

    int noAddUnits = stoi(argv[2]);
    int noMultUnits = stoi(argv[3]);
    int noBranchUnits = stoi(argv[4]);
    int noMemoryUnits = stoi(argv[5]);
    int memoryCycle = stoi(argv[6]);
    int adderCycle = stoi(argv[7]);
    int multCycle = stoi(argv[8]);
    int branchCycle = stoi(argv[9]);
    int issueCount = stoi(argv[10]);
    int commitCount = stoi(argv[11]);
    bool isBranchTaken = (trueString.compare(argv[12]) == 0);
    bool isSpeculative = (trueString.compare(argv[13]) == 0);

    // TomsuloSimulator tm(instrArray, 3, 2, 2, 5, 1, 1, 2, 1, 2, 2,false);
    TomsuloSimulator tm(instrArray, noAddUnits, noMultUnits, noBranchUnits, noMemoryUnits, memoryCycle, adderCycle, multCycle, branchCycle, issueCount, commitCount, isBranchTaken, isSpeculative);
    tm.execute();
    tm.printTimingCycle();
}
