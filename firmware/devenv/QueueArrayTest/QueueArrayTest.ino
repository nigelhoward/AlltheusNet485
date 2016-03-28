#include <AllQueue.h>
/*
 * QueueArrayTest.ino
 *
 * Created: 3/6/2016 11:57:15 AM
 * Author: nigel
 */ 


struct Employee
{
	unsigned long Id;
	int age;
	double wage;
	byte employeeHistory[10];

};

AllQueue <Employee> employees;

long startMillis;

void setup()
{

	employees.maxQueueSize = 2; // For example - Will be overwritten in init below and set to init value
	employees.init(15);
	employees.resizeQueueBy = 1;

	//employees.maxQueueSize = 12; // NOT overwritten now

	startMillis = millis();
	Serial.begin(115200);

	Serial.print(" Byte size of queue:");
	Serial.println(sizeof(Employee) * employees.size);
	Serial.println();

	Serial.print("Employees in que:");
	Serial.print(employees.count());

	Serial.print(" Size of Employee:");
	Serial.println(sizeof(Employee));
	Serial.println();

	Serial.println("Hello - Let's create an employee list");
	Serial.println();

	for (int i = 0; i <= 8; i++)
	{

		Serial.print("Index:");
		Serial.print(i);
		Serial.print(" ");
		tryEmploySomeone(i+1);

	}
	Serial.println();
	
	int acount = employees.count();
	
	for (int i=0; i< acount;i++)
	{
		
		Employee employee = employees.getByQueuePosition(i);

		Serial.print("Index:");
		Serial.print(i);
		Serial.print(" EmpId:");
		Serial.print(employee.Id);
		Serial.print(" Age=");
		Serial.println(employee.age);
	}
	
	Serial.println("");
	Serial.println("end of new list");
	Serial.println("");
	
}

void tryEmploySomeone(int id)
{
	Employee newEmp;
	newEmp.age = random(18, 56);
	newEmp.Id = id;
	newEmp.wage = random(2500, 5500);

	if (employees.enqueue(newEmp))
	{
		Serial.print("EmpId:");
		Serial.print(id);
		Serial.print(" Age:");
		Serial.print(newEmp.age);
		Serial.println(" was hired");
	}
	else
	{
		Serial.print("EmpId:");
		Serial.print(id);
		Serial.println(" not hired!");

	}
}

void loop()
{
	
	Serial.println();
	Serial.print("We have ");
	Serial.print(employees.count());
	Serial.print(" employees");
	Serial.println();

	int counter = 0;

	//int foundId = employees.findByLongId(queuePosition);
	//Employee findEmp;
	//if(foundId==-9) Serial.println("No one there at position ");
	//findEmp = employees.getByQueuePosition(foundId);
	//Serial.print("Found EmpId:");
	//Serial.print(findEmp.Id);
	//Serial.print(" Age=");
	//Serial.println(findEmp.age);

	int empIdToDelete = random(1,10);
	Serial.println();
	Serial.print("Delete employee Id:");
	Serial.println(empIdToDelete);
	
	if(!employees.deleteByLongId(empIdToDelete)) 
	{
		Serial.println("No such employee Id");
	}
	else
	{
		Serial.println("Deleted");
		Serial.println();
		
	}

	
	
	int qcount = employees.count();
	if(qcount==0) while(0==0){}
	
	for (int i=0; i< qcount;i++)
	{
		
		Employee employee = employees.dequeue();
		Serial.print("Index:");
		Serial.print(i);
		Serial.print(" EmpId:");
		Serial.print(employee.Id);
		Serial.print(" Age=");
		Serial.println(employee.age);
		employees.enqueue(employee);
	}
	
	//delay(500);
	return;

	while (!employees.isEmpty())
	{
		//delay((800 / employees.count())); // Pause and take stock :-)

		Employee employee = employees.dequeue();

		Serial.print("EmpId:");
		Serial.print(employee.Id);
		Serial.print(" Age=");
		Serial.print(employee.age);
		
		if (employee.age > 64)
		{
			Serial.println(" retired. Goodbye!");
		}
		else
		{
			Serial.print(" working.");;

			employee.age++;
			employees.enqueue(employee);

			Serial.print(" We have ");
			Serial.print(employees.count());
			Serial.println(" employees");


		}

		counter++;
		if (counter<9000)
		{
			if(random(1,32) ==1 )tryEmploySomeone(counter);
		}
		if (counter==9000)
		{
			Serial.println();
			Serial.println(" We stopped hiring people");
			Serial.println();

		}
	}
	Serial.println();
	Serial.println(" We closed the company :-(");
	Serial.println();

	Serial.println();
	Serial.print("Employees left :");
	Serial.println(employees.count());

	Serial.print("Final queue capacity :");
	Serial.println(employees.size);


	Serial.print("Final byte size of queue:");
	Serial.println(sizeof(Employee) * employees.size);
	Serial.println();

	Serial.print("Finished :");
	Serial.print(counter);
	Serial.print(" cycles in :");
	Serial.print(millis()- startMillis);
	Serial.print(" Millis");

	while (true != false)
	{

	}

}


