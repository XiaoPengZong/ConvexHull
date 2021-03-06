#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct point_t {
	double x;
	double y;
} point_t;

// получение массива из n точек
point_t* getpoints(FILE *f, int *n) {
	int capacity = 1;
	*n = 0;
	point_t *points = malloc(capacity * sizeof(point_t));
	point_t p;

	while (fscanf(f, "%lf %lf", &p.x, &p.y) == 2) {
		points[(*n)++] = p;

		if (*n >= capacity) {
			capacity *= 2;
			points = (point_t *) realloc(points, capacity * sizeof(point_t)); 
		}
	} 

	return points;
}

// вывод на экран массива точек
void print_points(point_t *points, int n) {
	for (int i = 0; i < n; i++)
		printf("(%lf, %lf)\n", points[i].x, points[i].y);

	printf("\n");
}

// с какой стороны от массива ab находится точка c (>0 — левая сторона, < 0 — правая).
int rotate(point_t a, point_t b, point_t c) {
	return (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
}

// расстояние от точки p до прямой ab
double distanceToLine(point_t p, point_t a, point_t b) {
	return fabs((b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y)) / sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}

point_t *copy_points(point_t *points, int n) {
	point_t *copy = (point_t *) malloc(n * sizeof(point_t));

	for (int i = 0; i < n; i++)
		copy[i] = points[i];

	return copy;
}

// построение минимальной выпуклой оболчки по алгоритму Грэхэма
point_t* graham_hull(point_t *points, int n, int *hull_n) {
	points = copy_points(points, n); // копируем точки

	// определяем самую левую точку
	for (int i = 1; i < n; i++) {
		if (points[i].x < points[0].x) {
			point_t tmp = points[i];
			points[i] = points[0];
			points[0] = tmp;
		}
	}

	// сортировка всех точек по степени "левизны" относительно стартовой точки
	for (int i = 2; i < n; i++) {
		int j = i;

		while (j > 1 && rotate(points[0], points[j-1], points[j]) < 0) {
			point_t tmp = points[j];
			points[j] = points[j - 1];
			points[j - 1] = tmp;
			j--;
		}
	}

	point_t *S = (point_t *) malloc(n * sizeof(point_t));

	*hull_n = 0;
	S[(*hull_n)++] = points[0];
	S[(*hull_n)++] = points[1];

	// срезание углов (удаление вершин с правым поворотом)
	for (int i = 2; i < n; i++) {
		while (rotate(S[*hull_n - 2], S[*hull_n - 1], points[i]) <= 0)
			(*hull_n)--;

		S[(*hull_n)++] = points[i];
	}

	free(points);

	return S;
}

// построение минимальной выпуклой оболчки по алгоритму Джарвиса
point_t* jarvis_hull(point_t *points, int n, int *hull_n) {
	points = copy_points(points, n); // копируем точки

	// определяем самую левую точку
	for (int i = 1; i < n; i++) {
		if (points[i].x < points[0].x) {
			point_t tmp = points[i];
			points[i] = points[0];
			points[0] = tmp;
		}
	}

	point_t *H = (point_t *) malloc(n * sizeof(point_t));

	*hull_n = 0;
	H[(*hull_n)++] = points[0];

	for (int i = 1; i < n; i++)
		points[i - 1] = points[i];

	points[n - 1] = H[0];

	while (1) {
		int right = 0;

		for (int i = 1; i < n; i++) {
			if (rotate(H[*hull_n - 1], points[right], points[i]) < 0)
        		right = i;
		}

		if (points[right].x == H[0].x && points[right].y == H[0].y)
			break;
		else {
			H[(*hull_n)++] = points[right];

			for (int i = right + 1; i < n; i++)
				points[i - 1] = points[i];
			
			n--;
		}
	}

	free(points);

	return H;
}

// построение минимальной выпуклой оболчки по алгоритму Эндрю
point_t* andrew_hull(point_t *points, int n, int *hull_n) {
	points = copy_points(points, n);

	point_t *hull = (point_t *) malloc(2 * n * sizeof(point_t));
	*hull_n = 0;

	// сортируем точки по удалённости
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n - 1; j++) {
			if ((points[j].x >= points[j + 1].x) && (points[j].x != points[j + 1].x || points[j].y >= points[j + 1].y)) {
				point_t p = points[j];
				points[j] = points[j + 1];
				points[j + 1] = p;
			}
		}
	}

	// строим нижнюю часть оболочки
	for (int i = 0; i < n; i++) {
		while (*hull_n >= 2 && rotate(hull[*hull_n - 2], hull[*hull_n - 1], points[i]) <= 0)
			(*hull_n)--;

		hull[(*hull_n)++] = points[i];
	}

	// строим верхнюю часть оболочки
	for (int i = n - 1, t = *hull_n + 1; i > 0; i--) {
		while (*hull_n >= t && rotate(hull[*hull_n - 2], hull[*hull_n - 1], points[i - 1]) <= 0)
			(*hull_n)--;

		hull[(*hull_n)++] = points[i - 1];
	}

	(*hull_n)--;

	free(points);

	return hull;
}

// разделение точек на два подмножества для алгоритмы быстрой выпуклой оболочки
point_t* divide(point_t *points, int n, point_t p1, point_t p2, int *hull_n) {
	point_t *hull = NULL;
	*hull_n = 0;

	if (n == 0)
		return NULL;

	if (n == 1) {
		hull = (point_t *) malloc(sizeof(point_t));
		hull[0] = points[0];
		*hull_n = 1;

		return hull;
	}

	points = copy_points(points, n);

	point_t maxDistancePoint = points[0];
	int index = 0;
	double distance = 0.0;

	for (int i = 0; i < n; i++) {
		if (distanceToLine(points[i], p1, p2) > distance) {
			distance = distanceToLine(points[i], p1, p2);
			maxDistancePoint = points[i];
			index = i;
		}
	}

	for (int i = index + 1; i < n; i++)
		points[i - 1] = points[i];
	
	n--;


	point_t *l1 = (point_t *) malloc(n * sizeof(point_t));
	point_t *l2 = (point_t *) malloc(n * sizeof(point_t));

	int l1_n = 0;
	int l2_n = 0;

	for (int i = 0; i < n; i++) {
		if (rotate(points[i], p1, maxDistancePoint) <= 0) {
			l1[l1_n++] = points[i];
		}
		else if (rotate(points[i], maxDistancePoint, p2) <= 0) {
			l2[l2_n++] = points[i];
		}
	}

	int bottom_n, top_n;

	point_t *bottom = divide(l1, l1_n, p1, maxDistancePoint, &bottom_n);
	point_t *top = divide(l2, l2_n, maxDistancePoint, p2, &top_n);

	hull = (point_t *) malloc((bottom_n + 1 + top_n) * sizeof(point_t));

	*hull_n = 0;
	for (int i = 0; i < bottom_n; i++)
		hull[(*hull_n)++] = bottom[i];

	hull[(*hull_n)++] = maxDistancePoint;

	for (int i = 0; i < top_n; i++)
		hull[(*hull_n)++] = top[i];

	free(l1);
	free(l2);
	free(bottom);
	free(top);
	free(points);

	return hull;
}

// построение минимальной выпуклой оболчки по быстрому алгоритму
point_t* quick_hull(point_t *points, int n, int *convex_hull_n) {
	point_t *convex_hull = (point_t *) malloc(n * sizeof(point_t));
	*convex_hull_n = 0;

	point_t leftMostPoint = points[0];
	point_t rightMostPoint = points[0];

	// находим самую левую и самую првую точки
	for (int i = 1; i < n; i++) {
		if (points[i].x > rightMostPoint.x) {
			rightMostPoint = points[i];
		}
		else if (points[i].x < leftMostPoint.x) {
			leftMostPoint = points[i];
		}
	}

	point_t *leftOfLine = (point_t *) malloc(n * sizeof(point_t));
	point_t *rightOfLine = (point_t *) malloc(n * sizeof(point_t));

	int left_n = 0;
	int right_n = 0;

	// разделяем на два подмножества - выше прямой и ниже
	for (int i = 0; i < n; i++) {
		if ((points[i].x == rightMostPoint.x && points[i].y == rightMostPoint.y) || (points[i].x == leftMostPoint.x && points[i].y == leftMostPoint.y))
			continue;

		if (rotate(points[i], leftMostPoint, rightMostPoint) <= 0) {
			leftOfLine[left_n++] = points[i];
		}
		else {
			rightOfLine[right_n++] = points[i];
		}
	}

	convex_hull[(*convex_hull_n)++] = leftMostPoint;

	int hull_n;
	point_t *hull = divide(leftOfLine, left_n, leftMostPoint, rightMostPoint, &hull_n);

	for (int i = 0; i < hull_n; i++)
		convex_hull[(*convex_hull_n)++] = hull[i];

	convex_hull[(*convex_hull_n)++] = rightMostPoint;

	free(hull);

	hull = divide(rightOfLine, right_n, rightMostPoint, leftMostPoint, &hull_n);

	for (int i = 0; i < hull_n; i++)
		convex_hull[(*convex_hull_n)++] = hull[i];

	free(hull);
	free(leftOfLine);
	free(rightOfLine);

	return convex_hull;
}

int main() {
	char path[100];
	printf("Enter path: ");
	scanf("%s", path);

	FILE *f = fopen(path, "r");

	if (!f) {
		printf("Error during opening file '%s'\n", path);
		return -1;
	}

	int n;
	point_t *points = getpoints(f, &n);
	fclose(f);

	printf("Readed points:\n");
	print_points(points, n);

	int graham_n;
	point_t *graham_points = graham_hull(points, n, &graham_n);
	printf("Graham hull points:\n");
	print_points(graham_points, graham_n);

	int jarvis_n;
	point_t *jarvis_points = jarvis_hull(points, n, &jarvis_n);
	printf("Jarvis hull points:\n");
	print_points(jarvis_points, jarvis_n);

	int andrew_n;
	point_t *andrew_points = andrew_hull(points, n, &andrew_n);
	printf("Andrew hull points:\n");
	print_points(andrew_points, andrew_n);

	int quick_n;
	point_t *quick_points = quick_hull(points, n, &quick_n);
	printf("Quick hull points:\n");
	print_points(quick_points, quick_n);

	free(points);
	free(graham_points);
	free(jarvis_points);
	free(andrew_points);
	free(quick_points);
}
