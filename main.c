#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "structs.h"

#define MAX_STR 30

#define DIE(assertion, call_description)                \
	do {                                \
		if (assertion) {                    \
			fprintf(stderr, "(%s, %d): ",           \
					__FILE__, __LINE__);        \
			perror(call_description);           \
			exit(errno);                        \
		}                           \
	} while (0)

// typedef-ul asta e ca sa prescurtez numele, sa fie mai usor la coding style,
// pana la urma arata si codul mai frumos
typedef power_management_unit PMU_t;

// typedef-ul asta e pentru ca nu accepta checker-ul int8_t (si m-am obinsuit
// de la sda sa  il folosesc )si aveam nevoie de o prescurtare la signed char,
//pentru coding style
typedef signed char s8_t;

extern void get_operations(void **operations);

// functie care pune in senzorul de tip TIRE, datele trimise ca parametrii
void populate_tire_sensor(tire_sensor *sensor, float pressure, float temp,
						  int wear_level, int performance_score)
{
	sensor->pressure = pressure;
	sensor->temperature = temp;
	sensor->wear_level = wear_level;
	sensor->performace_score = performance_score;
}

// functie care pune in senzorul de tip PMU, datele trimise ca parametrii
void populate_pmu(PMU_t *sensor, float voltage, float current,
				  float power_consumption, int energy_regen,
				  int energy_storage)
{
	sensor->voltage = voltage;
	sensor->current = current;
	sensor->power_consumption = power_consumption;
	sensor->energy_regen = energy_regen;
	sensor->energy_storage = energy_storage;
}

// functie care citeste din fisierul binar vectorul de senzori
sensor *read_sensor_array(FILE *binary_file)
{
	// citesc cate elemente trebuie sa aibe vectorul
	int how_many = 0;
	fread(&how_many, sizeof(int), 1, binary_file);

	sensor *s_arr = (sensor *)malloc(how_many * sizeof(sensor));
	DIE(!s_arr, "Error while creating the array of sensors.\n");

	// populez vectorul cu senzori / date
	for (int i = 0; i < how_many; i++) {
		int type;

		// citesc tipul senzorului
		fread(&type, sizeof(int), 1, binary_file);
		s_arr[i].sensor_type = type;

		// citesc datele senzorului si le pun in vector
		switch (s_arr[i].sensor_type) {
		case TIRE:
			// aloc sensor_data
			s_arr[i].sensor_data = (tire_sensor *)malloc(sizeof(tire_sensor));
			DIE(!s_arr[i].sensor_data, "Error while creating sensor data.\n");

			float press, temp;
			int level, score;
			fread(&press, sizeof(float), 1, binary_file);
			fread(&temp, sizeof(float), 1, binary_file);
			fread(&level, sizeof(int), 1, binary_file);
			fread(&score, sizeof(int), 1, binary_file);

			// pun datele citite in senzor
			populate_tire_sensor((tire_sensor *)s_arr[i].sensor_data, press,
								 temp, level, score);
			break;
		case PMU:
			// aloc sensor_data
			s_arr[i].sensor_data = (PMU_t *)malloc(sizeof(PMU_t));
			DIE(!s_arr[i].sensor_data, "Error while creating sensor data.\n");

			float volt, curr, power;
			int regen, storage;
			fread(&volt, sizeof(float), 1, binary_file);
			fread(&curr, sizeof(float), 1, binary_file);
			fread(&power, sizeof(float), 1, binary_file);
			fread(&regen, sizeof(int), 1, binary_file);
			fread(&storage, sizeof(int), 1, binary_file);

			// pun datele citite in senzor
			populate_pmu((PMU_t *)s_arr[i].sensor_data, volt, curr, power,
						 regen, storage);
			break;
		default:
			break;
		}

		// citesc numarul de operatii
		int op_num;
		fread(&op_num, sizeof(int), 1, binary_file);
		s_arr[i].nr_operations = op_num;

		// aloc vectorul de operatii
		s_arr[i].operations_idxs = (int *)malloc(op_num * sizeof(int));
		DIE(!s_arr[i].operations_idxs, "Error while creating sensor data.\n");

		// populez si vectorul de operatii
		for (int j = 0; j < op_num; j++)
			fread(&s_arr[i].operations_idxs[j], sizeof(int), 1, binary_file);
	}

	return s_arr;
}

// functie care printeaza datele unui sensor de tip TIRE
void print_tire(tire_sensor *my_sensor)
{
	printf("Tire Sensor\n");
	printf("Pressure: %.2lf\n", my_sensor->pressure);
	printf("Temperature: %.2lf\n", my_sensor->temperature);
	printf("Wear Level: %d%%\n", my_sensor->wear_level);
	printf("Performance Score: ");

	if (my_sensor->performace_score == 0) {
		printf("Not Calculated\n");
		return;
	}

	// ajunge aici doar daca n-a intrat in if
	printf("%d\n", my_sensor->performace_score);
}

// functie care printeaza datele unui sensor de tip PMU
void print_pmu(power_management_unit *my_sensor)
{
	printf("Power Management Unit\n");
	printf("Voltage: %.2lf\n", my_sensor->voltage);
	printf("Current: %.2lf\n", my_sensor->current);
	printf("Power Consumption: %.2lf\n", my_sensor->power_consumption);
	printf("Energy Regen: %d%%\n", my_sensor->energy_regen);
	printf("Energy Storage: %d%%\n", my_sensor->energy_storage);
}

// functie care intoarce un vector "de pozitii"
int *rearrange_sensors(sensor *arr, int arr_dim)
{
	// ideea mea este de a lasa vectorul nemodificat, si sa retin intr-un
	// vector indecsii "reali", care ar fi rezultat in urma unei sortari

	// aloc vectorul de pozitii
	int *idx_arr = (int *)malloc(arr_dim * sizeof(int));
	DIE(!idx_arr, "Error while rearranging the sensors.\n");

	// cu i parcurg vectorul de pozitii
	int i = 0;

	// intai parcurg vectorul de senzori, uitandu-ma doar dupa senzori de tip
	// PMU, si pun index-ul in vectorul de pozitii
	for (int j = 0; j < arr_dim; j++) {
		if (arr[j].sensor_type == PMU) {
			idx_arr[i] = j;
			i++;
		}
	}

	// mai parcurg odata vectorul, uitandu-ma doar dupa senzori de tip TIRE, si
	// le pun index-ul in vectoruk de pozitii
	for (int j = 0; j < arr_dim; j++) {
		if (arr[j].sensor_type == TIRE) {
			idx_arr[i] = j;
			i++;
		}
	}

	return idx_arr;
}

// functie care printeaza senzorul de pe pozitia idx, dar pozitia este cea care
// ar fi rezultat in urma sortarii, retiunta in vectorul de pozitii pos
void print_sensor(sensor *arr, int *pos, int arr_dim, int idx)
{
	// un index invalid
	if (idx < 0 || idx >= arr_dim) {
		printf("Index not in range!\n");
		return;
	}

	// index-ul real din vectorul de senzori
	idx = pos[idx];
	if (arr[idx].sensor_type == TIRE) {
		tire_sensor *data = (tire_sensor *)arr[idx].sensor_data;
		print_tire(data);
		return;
	}

	// ajunge aici doar daca n-a intrat in if, deci daca este de tip PMU
	PMU_t *data = (PMU_t *)arr[idx].sensor_data;
	print_pmu(data);
}

// functie care aplica operatiile pe un senzor
void apply_operations(sensor *my_sensor, void (*funcs[8])(void *))
{
	// parcurg vectorul de operatii si apelez operatia coresunzatoare
	int n = my_sensor->nr_operations;
	int op_idx;
	for (int i = 0; i < n; i++) {
		op_idx = my_sensor->operations_idxs[i];
		funcs[op_idx](my_sensor->sensor_data);
	}
}

// functie care face efectiv comanda de analyze
void analyze_sensor(sensor *sens_arr, int *pos, int arr_dim, int idx,
					void (*funcs[8])(void *))
{
	// verific daca indexul este valid
	if (idx < 0 || idx > arr_dim) {
		printf("Index not in range!\n");
		return;
	}

	// obtin indexul real, din vectorul de senzori
	idx = pos[idx];
	sensor *curr_sensor = &sens_arr[idx];
	apply_operations(curr_sensor, funcs);
}

// functie care verifica daca senzorul TIRE este in parametrii normali
s8_t check_tire(tire_sensor *my_sensor)
{
	if (my_sensor->pressure < 19 || my_sensor->pressure > 28)
		return 0;

	if (my_sensor->temperature < 0 || my_sensor->temperature > 120)
		return 0;

	if (my_sensor->wear_level < 0 || my_sensor->wear_level > 100)
		return 0;

	return 1;
}

// functie care verifica daca senzorul PMU este in parametrii normali
s8_t check_pmu(power_management_unit *my_sens)
{
	if (my_sens->voltage < 10 || my_sens->voltage > 20)
		return 0;

	if (my_sens->current < -100 || my_sens->current > 100)
		return 0;

	if (my_sens->power_consumption < 0 || my_sens->power_consumption > 1000)
		return 0;

	if (my_sens->energy_regen < 0 || my_sens->energy_regen > 100)
		return 0;

	if (my_sens->energy_storage < 0 || my_sens->energy_storage > 100)
		return 0;

	return 1;
}

// functie care copiaza senzorul de la adresa src, la adresa dest (un fel de
// memcpy pentru senzori)
void copy_sensor(sensor *dest, sensor *src)
{
	enum sensor_type type;
	type = src->sensor_type;

	// ii dau destinatiei noul tip
	dest->sensor_type = type;

	free(dest->sensor_data);
	if (type == TIRE) {
		// aloc sensor_data al destinatiei
		dest->sensor_data = (tire_sensor *)malloc(sizeof(tire_sensor));
		DIE(!dest->sensor_data, "Error while reallocating a sensor\n");

		// copiez din sursa in destinatie
		memcpy(dest->sensor_data, src->sensor_data, sizeof(tire_sensor));
	} else {
		// aloc sensor_data al destinatiei
		dest->sensor_data = (PMU_t *)malloc(sizeof(PMU_t));
		DIE(!dest->sensor_data, "Error while reallocating a sensor\n");

		// copiez din sursa in destinatie
		memcpy(dest->sensor_data, src->sensor_data, sizeof(PMU_t));
	}

	// procedez asemanator si cu vectorul de operatii
	int nr = src->nr_operations;
	dest->nr_operations = nr;
	free(dest->operations_idxs);

	dest->operations_idxs = (int *)malloc(nr * sizeof(int));
	DIE(!dest->operations_idxs, "Error while reallocating a sensor\n");

	memcpy(dest->operations_idxs, src->operations_idxs, nr * sizeof(int));
}

// functie care sterge senzorul de la pozitia idx, din vectorul de senzori
// nu se modifica nimic in vectorul de pozitii, ci se reapeleaza functia deja
// facuta
void remove_sensor(sensor **array, int arr_dim, int idx)
{
	// mut toti senzorii la index-ul precedent, de la idx in colo
	for (int i = idx; i < arr_dim - 1; i++) {
		sensor *dest = *array + i;
		sensor *src = *array + i + 1;
		copy_sensor(dest, src);
	}

	// sterg ultimul senzor care e prezent de 2 ori
	free((*array + arr_dim - 1)->sensor_data);
	free((*array + arr_dim - 1)->operations_idxs);
	*array = (sensor *)realloc(*array, (arr_dim - 1) * sizeof(sensor));
	DIE(!(*array), "Error while reallocating the array\n");
}

// sterge un element dintr-un vector care imita tipul boolean (de aceea
// folosesc si signed char, nu am nevoie de mai mult decat 1 si 0)
void remove_element(s8_t **array, int arr_dim, int idx)
{
	for (int i = idx; i < arr_dim - 1; i++)
		*(*array + i) = *(*array + i + 1);

	*array = (s8_t *)realloc(*array, (arr_dim - 1) * sizeof(s8_t));
	DIE(!(*array), "Error while reallocating the array\n");
}

// functie care scoate din vector senzorii eronati
void clear_sensors(sensor **array, int **pos, int *arr_dim)
{
	sensor *curr_sensor;
	// aloc un vector in care bifez cu 0 senzorii care trebuie stersi
	s8_t *check = (s8_t *)malloc(*arr_dim * sizeof(s8_t));
	DIE(!check, "Error while deleting from array\n");

	int i;

	// completez vectorul check
	for (i = 0; i < *arr_dim; i++) {
		curr_sensor = *array + i;
		if (curr_sensor->sensor_type == TIRE)
			check[i] = check_tire((tire_sensor *)curr_sensor->sensor_data);
		else
			check[i] = check_pmu((PMU_t *)curr_sensor->sensor_data);
	}

	// sterg senzorii marcati
	i = 0; // plec de pe prima pozitie
	while (i < *arr_dim) {
		// daca trebuie sters, il sterg si raman pe aceeasi pozitie
		// (s-ar putea ca noul senzor de pe pozitia actuala sa trebuiasca
		// sa fie sters)
		if (check[i] == 0) {
			remove_sensor(array, *arr_dim, i);
			remove_element(&check, *arr_dim, i);
			*arr_dim = *arr_dim - 1;
			continue;
		}

		// altfel cresc pozitia (inseamna ca senzorul nu trebuie sters)
		i++;
	}

	// rearanjez pozitiile
	free(*pos);
	*pos = rearrange_sensors(*array, *arr_dim);
	DIE(!(*pos), "Error while deleting from array\n");

	free(check);
}

// parser de comenzi
s8_t get_task(char string[MAX_STR])
{
	if (strcmp(string, "print") == 0)
		return 1;

	if (strcmp(string, "analyze") == 0)
		return 2;

	if (strcmp(string, "clear") == 0)
		return 3;

	if (strcmp(string, "exit") == 0)
		return 0;

	return -1;
}

// functie care elibereaza memoria folosita de vectorul de senzorii
void free_sensors(sensor **array, int array_dim)
{
	// pentru fiecare senzor eliberez intai data si vectorul de operatii
	for (int i = 0; i < array_dim; i++) {
		free((*array + i)->sensor_data);
		free((*array + i)->operations_idxs);
	}

	// eliberez vectorul si-l setez pe NULL (ca asa am invatat la sd ca e bine)
	free(*array);
	*array = NULL;
}

int main(int argc, char const *argv[])
{
	// deschid fisierul binar
	FILE *binary_file = fopen(argv[1], "rb");
	DIE(!binary_file, "Error while opening the input file.\n");

	// creez vectorul de senzori
	sensor *sensor_array = NULL;
	sensor_array = read_sensor_array(binary_file);

	// ma intorc la inceputul fisierului ca sa mai citesc odata numarul
	// de senzori (a fost citit odata in functie, dar nu a fost returnat)
	int array_dim = 0;
	fseek(binary_file, 0, SEEK_SET);
	fread(&array_dim, sizeof(int), 1, binary_file);

	// inchid fisierul
	fclose(binary_file);

	// creez vectorul de operatii
	void (*funcs[8])(void *);
	get_operations((void **)funcs);

	// creez vectorul de operatii
	int *positions  = rearrange_sensors(sensor_array, array_dim);

	// parsez comenzile
	s8_t task = 0;
	int idx;
	char string[MAX_STR];
	do {
		scanf("%s", string);
		task = get_task(string);
		switch (task) {
		case 1:
			scanf("%d", &idx);
			print_sensor(sensor_array, positions, array_dim, idx);
			break;
		case 2:
			scanf("%d", &idx);
			analyze_sensor(sensor_array, positions, array_dim, idx, funcs);
			break;
		case 3:
			clear_sensors(&sensor_array, &positions, &array_dim);
			break;
		case 0:
			break;
		default:
			printf("Invalid command!\n");
			break;
		}
	} while (task != 0);

	// eliberez resursele folosite in main
	free_sensors(&sensor_array, array_dim);
	free(positions);
	return 0;
}
