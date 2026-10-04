#include <iostream>
using namespace std;

struct Task {
    int ID;
    double p; //czas wykonania
};

struct TaskList {
    Task *array;
    int capacity; //ile zadań w jednym momencie
    int size; //ile zadań jest w trakcie

    //constructor
    TaskList() {
        capacity = 2;
        size = 0;
        array = new Task[capacity];
    }

    ~TaskList() {
        delete[] array;
    }

    //copy constructor
    TaskList(const TaskList &other) {
        capacity = other.capacity;
        size = other.size;
        array = new Task[capacity];
        for (int i = 0; i < size; i++) {
            array[i] = other.array[i];
        }
    }
    //operator przypisania
    TaskList &operator=(const TaskList &other) {
        if (this != &other) {
            delete[] array;
            capacity = other.capacity;
            size = other.size;
            array = new Task[capacity];
            for(int i = 0; i < size; i++) {
                array[i] = other.array[i];
            }
        }
        return *this;
    }

    void addTaskAtPosition(int index, const Task &newTask) {
        if (size == capacity) {
            capacity = capacity * 2;
            Task *new_array = new Task[capacity];

            for (int i = 0; i < size; i++) {
                new_array[i] = array[i];
            }

            delete[] array;
            array = new_array;
        }

        //przesuwanie zadań od końca w prawą stronę
        for (int i = size; i > index; i--) {
            array[i] = array[i - 1];
        }

        array[index] = newTask;
        size++;
    }

    void removeTaskByID(int idToRemove) {
        int indexToRemove = -1; //not found yet

        for (int i = 0; i < size; i++) {
            if (array[i].ID == idToRemove) {
                indexToRemove = i;
                break;
            }
        }

        if (indexToRemove != -1) {
            for (int j = indexToRemove; j < size-1; j++) {
                array[j] = array[j+1];
            }
            size--;
        }
    }
};

struct Machine {
    double freeTime; //godzina o której skończy obecną prace 

    int *historyID;
    double *historyC; //time history
    int tasksCount; //ile zadań dostała
};

void printAndCleanup(Machine *machines, int m, double sigmaC) {
    double Cmax = 0.0; 
    //szukanie najwyższej wartości czasu
    for (int i = 0; i < m; i++) {
        if(machines[i].freeTime > Cmax) {
            Cmax = machines[i].freeTime;
        }
    }

    printf("Cmax: %g\n", Cmax);
    printf("sigmaC: %g\n", sigmaC);

    for (int i = 0; i < m; i++) {
            printf("M%d:", i+1);

        for (int j = 0; j < machines[i].tasksCount; j++) {
            int currentID = machines[i].historyID[j];
            double currentC = machines[i].historyC[j];

            bool byloCiete = false;
            //jeśli to pierwsze zadanie na nowej maszynie i jego id jest takie samo
            //jak ostatniego zadania na poprzedniej, to było cięte
            if (i > 0 && j == 0) {
                int lastIndexCut = machines[i-1].tasksCount - 1;
                if (lastIndexCut >= 0 && machines[i-1].historyID[lastIndexCut] == currentID) {
                    byloCiete = true;
                }
            }
                
            if (byloCiete) printf("( P%d = %g )", currentID, currentC);
            else printf("( C%d = %g )", currentID, currentC);
        }
        printf("\n"); 
    }

    //memory cleaning
    for (int i = 0; i < m; i++) {
        delete[] machines[i].historyID;
        delete[] machines[i].historyC;
    }
    delete[] machines;
}

void assignTaskForBL(Machine *machines, int m, Task currentTask, double &sigmaC) {
    //looking for the proper machine
    int bestMachineIndex = 0;
    for (int j = 0; j < m; j++) {
        if (machines[j].freeTime < machines[bestMachineIndex].freeTime) {
            bestMachineIndex = j;
        }
    }

    machines[bestMachineIndex].freeTime += currentTask.p;
    double finishTime = machines[bestMachineIndex].freeTime;
    sigmaC += finishTime;

    //machies history and memory
    int printedIndex = machines[bestMachineIndex].tasksCount; //id pod którym zapisujemy nowe zadanie
    machines[bestMachineIndex].historyID[printedIndex] = currentTask.ID; //numer zadania zapisanego pod konkretnym id
    machines[bestMachineIndex].historyC[printedIndex] = finishTime; //czas po którym konkretne zadanie sie skończy
    machines[bestMachineIndex].tasksCount++;
}

//---------------------B---------------------------------------------
void scheduleB(TaskList &tasks, int m) {
    if (tasks.size == 0) return; 

    Machine *machines = new Machine[m];

    for (int i = 0; i < m; i++) {
        machines[i].freeTime = 0.0;
        machines[i].tasksCount = 0;

        machines[i].historyID = new int[tasks.size];
        machines[i].historyC = new double[tasks.size];
    }

    double sigmaC = 0.0; //suma wszystkich czasów zakończenia

    for (int i = 0; i < tasks.size; i++) {
        Task currentTask = tasks.array[i];
        assignTaskForBL(machines, m, currentTask, sigmaC);
    }
    
    printAndCleanup(machines, m, sigmaC);
}

//---------------------------------SORTING-----------------
void swapTasks(Task &a, Task &b) {
    Task temp = a;
    a = b;
    b = temp;
}
bool isMoreImportant(Task a, Task b) {
    if (a.p > b.p) return true;
    if (a.p == b.p && a.ID > b.ID) return true;
    return false;
}
void heapify(Task *arr, int n, int i) {
    int parent = i;
    int left = 2*i+1;
    int right = 2*i+2;

    if ((left < n) && isMoreImportant(arr[left], arr[parent])) {
        parent = left;
    }
    if ((right < n) && isMoreImportant(arr[right], arr[parent])) {
        parent = right;
    }
    if (parent != i) {
        swapTasks(arr[i], arr[parent]);
        heapify(arr, n, parent);
    }
}
void heapSort(Task *arr, int n) {
    for (int i = n/2-1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    for (int i = n-1; i>0; i--) {
        swapTasks(arr[0], arr[i]);
        heapify(arr, i, 0);
    }
}

//------------------L----------------
void scheduleL(TaskList &tasks, int m) {
    if (tasks.size == 0) return;

    //osobna kopia zadań do posortowania
    Task *sortedTasks = new Task[tasks.size];
    for (int i = 0; i < tasks.size; i++) {
        sortedTasks[i] = tasks.array[i];
    }

    heapSort(sortedTasks, tasks.size); //sortowanie rosnące

    Machine *machines = new Machine[m];
    for(int i = 0; i < m; i++) {
        machines[i].freeTime = 0.0;
        machines[i].tasksCount = 0;
        machines[i].historyID = new int[tasks.size];
        machines[i].historyC = new double[tasks.size];
    }

    double sigmaC = 0.0;
    //ustawianie od najdłuższego zadania
    for (int i = tasks.size -1; i>=0; i--) {
        Task currentTask = sortedTasks[i];

        assignTaskForBL(machines, m, currentTask, sigmaC);
    }

    printAndCleanup(machines, m, sigmaC);
    delete[] sortedTasks;
}

//--------------S-----------------------
void scheduleS(TaskList &tasks, int m) {
    if (tasks.size == 0) return;

    //sortowanie - najdłuższe na koniec 
    Task *sortedTasks = new Task[tasks.size];
    for (int i = 0; i < tasks.size; i++) {
        sortedTasks[i] = tasks.array[i];
    }
    heapSort(sortedTasks, tasks.size);

    Machine *machines = new Machine[m];
    for (int i = 0; i < m; i++) {
        machines[i].freeTime = 0.0;
        machines[i].tasksCount = 0; //ile zadań ma do wykoanania
        machines[i].historyID = new int[tasks.size];
        machines[i].historyC = new double[tasks.size];
    }

    //poczekalnia zadań dla każdej maszyny
    Task **waiting = new Task*[m];
    for (int i = 0; i < m; i++) {
        waiting[i] = new Task[tasks.size];
    }

    //przydzielanie zadań do każdej maszyny
    int currentMachineIndex = 0;
    //czytanie zadań od najdłuższych
    for (int i = tasks.size - 1; i >= 0; i--) {
        waiting[currentMachineIndex][machines[currentMachineIndex].tasksCount] = sortedTasks[i];
        machines[currentMachineIndex].tasksCount++;
        currentMachineIndex++;
        if (currentMachineIndex == m) currentMachineIndex = 0;
    }

    double sigmaC = 0.0;
    for (int i = 0; i < m; i++) {
        heapSort(waiting[i], machines[i].tasksCount);
        int nrTasksWaiting = machines[i].tasksCount;
        machines[i].tasksCount = 0;

        for (int j = 0; j < nrTasksWaiting; j++) {
            Task currentTask = waiting[i][j];
            machines[i].freeTime += currentTask.p;
            sigmaC += machines[i].freeTime;
            machines[i].historyID[machines[i].tasksCount] = currentTask.ID;
            machines[i].historyC[machines[i].tasksCount] = machines[i].freeTime;
            machines[i].tasksCount++;
        }
        delete[] waiting[i];
    }

    printAndCleanup(machines, m, sigmaC);
    delete[] waiting;
    delete[] sortedTasks;
}

//----------------------M-----------------------------
void scheduleM(TaskList &tasks, int m) {
    if (tasks.size == 0) return;

    double sum_p = 0.0; //suma czasów wszytskich zdań
    double max_p = 0.0; //czas najdłuższego zadania

    for (int i = 0; i < tasks.size; i++) {
        Task currentTask = tasks.array[i];
        sum_p += currentTask.p;
        if (currentTask.p > max_p) max_p = currentTask.p;
    }

    //sprawdzamy czy maksymalny czas zadania mieści się w średniej
    double Cmax = sum_p / m;
    if (max_p > Cmax) Cmax = max_p;

    Machine *machines = new Machine[m];
    for (int i = 0; i < m; i++) {
        machines[i].freeTime = 0.0;
        machines[i].tasksCount = 0;

        machines[i].historyID = new int[tasks.size + m]; //więcej miejsca przez cięcie zadań
        machines[i].historyC = new double[tasks.size + m];
    }

    int currentMachine = 0;
    double busyMachineTime = 0.0;
    double sigmaC = 0.0;

    for (int i = 0; i < tasks.size; i++) {
        //pełna maszyna jest blokowana
        if(Cmax - busyMachineTime < 1e-7 && currentMachine < m - 1) {
            currentMachine++;
            busyMachineTime = 0.0;
        }

        Task currentTask = tasks.array[i];

        double freeSpace = Cmax - busyMachineTime; //wolne miejsce na obecnej maszynie

        //zadanie mieści się na maszynie
        if (currentTask.p <= freeSpace + 1e-7 || currentMachine == m-1) {
            busyMachineTime += currentTask.p;
            sigmaC += busyMachineTime;

            machines[currentMachine].historyID[machines[currentMachine].tasksCount] = currentTask.ID;
            machines[currentMachine].historyC[machines[currentMachine].tasksCount] = busyMachineTime;
            machines[currentMachine].freeTime = busyMachineTime;
            machines[currentMachine].tasksCount++;
        }
        //zadanie sie nie mieści
        else {
            machines[currentMachine].historyID[machines[currentMachine].tasksCount] = currentTask.ID;
            machines[currentMachine].historyC[machines[currentMachine].tasksCount] = Cmax;
            machines[currentMachine].freeTime = Cmax;
            machines[currentMachine].tasksCount++;
            currentMachine++;
            
            //reszta zadania na nowej maszynie
            double rest = currentTask.p - freeSpace;
            busyMachineTime = rest;

            sigmaC += Cmax;

            machines[currentMachine].historyID[machines[currentMachine].tasksCount] = currentTask.ID;
            machines[currentMachine].historyC[machines[currentMachine].tasksCount] = busyMachineTime;
            machines[currentMachine].freeTime = busyMachineTime;
            machines[currentMachine].tasksCount++;
        }
    }
    printAndCleanup(machines, m, sigmaC);
}

//------------------A--------------------------
void solveA(TaskList &tasks, int m, int currentTaskIndex, double *machineTimes, double &bestCmax) {
    //jeżeli wszytskie zadania przypisane
    if (currentTaskIndex == tasks.size) {
        double currentCmax = 0.0;
        for (int i = 0; i < m; i++) {
            if (machineTimes[i] > currentCmax) {
                currentCmax = machineTimes[i];
            }
        }

        if (currentCmax < bestCmax) {
            bestCmax = currentCmax;
        }
        return;
    }

    Task currentTask = tasks.array[currentTaskIndex];

    for (int i = 0; i < m; i++) {
        machineTimes[i] += currentTask.p;
        solveA(tasks, m, currentTaskIndex + 1, machineTimes, bestCmax);
        machineTimes[i] -= currentTask.p; //zdejmujemy zadanie z maszyny, żeby spróbowac położyć je na kolejnej
    }
}

void scheduleA(TaskList &tasks, int m) {
    if (tasks.size == 0) return;

    double bestCmax = 1e9; //bardzo duża liczba na początek

    double *machineTimes = new double[m];
    for (int i = 0; i < m; i++) {
        machineTimes[i] = 0.0;
    }

    solveA(tasks, m, 0, machineTimes, bestCmax);
    printf("Copt: %g\n", bestCmax);
    delete[] machineTimes;
}

//----------------------MAIN------------
int main() {
    int totalTasksCreated = 0;
    TaskList myTasks;

    int n;
    cin >> n;

    for (int i = 0; i < n; i++) {
        double p;
        cin >> p;

        totalTasksCreated++;

        Task newTask;
        newTask.ID = totalTasksCreated;
        newTask.p = p;

        myTasks.addTaskAtPosition(i, newTask);
    }

    char command;
    while (cin >> command) {
        
        if (command == '+') {
            int k;
            double p;
            cin >> k >> p;

            totalTasksCreated++;

            Task newTask;
            newTask.ID = totalTasksCreated;
            newTask.p = p;

            myTasks.addTaskAtPosition(k - 1, newTask);
        }
        else if (command == '-') {
            int idToRemove;
            cin >> idToRemove;

            myTasks.removeTaskByID(idToRemove);
        }
        else if (command == 'B') {
            int m;
            cin >> m;
            scheduleB(myTasks, m);
        }
        else if (command == 'L') {
            int m;
            cin >> m;
            scheduleL(myTasks, m);
        }
        else if (command == 'S') {
            int m;
            cin >> m;
            scheduleS(myTasks, m);
        }
        else if (command == 'M') {
            int m;
            cin >> m;
            scheduleM(myTasks, m);
        }
        else if (command == 'A') {
            int m;
            cin >> m;
            scheduleA(myTasks, m);
        }
    }

    return 0;
}
