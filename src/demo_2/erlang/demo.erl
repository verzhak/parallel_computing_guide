
%
% Многопоточное моделирование показательного распределения с помощью датчика псевдослучайных чисел, распределенных равномерно
%
% Запуск моделирования:
%
%	1. Запуск вычислительного узла amv_2@127.0.0.1:
%
%		1.1 Запуск виртуальной машины: erl -name amv_2@127.0.0.1
%		1.2 Компиляция модуля: c(demo).
%
%	2. Запуск вычислительного узла amv_1@127.0.0.1:
%
%		2.1 Запуск виртуальной машины: erl -name amv_1@127.0.0.1
%		2.2 Компиляция модуля: c(demo)
%
%	3. Запуск моделирования (вычислительный узел amv_1@127.0.0.1):
%
%		eval(TNum, Lambda, N)
%
%		Здесь:
%
%			TNum - количество вычислительных потоков (будут распределены равномерно между вычислительными узлами);
%			Lambda - параметр lambda показательного распределения;
%			N - количество генерируемых реализаций псевдослучайной величины.
%

-module(demo).
-import(math, [log / 1, pow / 2, sqrt / 1]).
-import(random, [seed / 1, uniform / 0]).
-import(lists, [foreach / 2, map / 2, seq / 2, split / 2, sum / 1]).
-export([eval / 3, thread_main / 3]).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Главная функция модуля
eval(TNum, Lambda, N) ->
	
	% Расчет количества реализаций псевдослучайной величины на поток
	N_per_thread = N div TNum,

	% Создание двух, примерно равных по количеству элементов, списков, каждый элемент которых содержит количество реализаций случайной величины, генерируемых соответствующим вычислительным потоком
	{ N_per_thread_1, N_per_thread_2 } = split(TNum div 2, map(fun(_) -> N_per_thread end, seq(1, TNum - 1)) ++ [N - (TNum - 1) * N_per_thread]),

	% Запуск вычислительных потоков
	Pid =
		map(fun(N_current_thread) -> spawn(demo, thread_main, [self(), Lambda, N_current_thread]) end, N_per_thread_1)
		++
		map(fun(N_current_thread) -> spawn('amv_2@127.0.0.1', demo, thread_main, [self(), Lambda, N_current_thread]) end, N_per_thread_2),

	% Расчет эмпирических математического ожидания и дисперсии
	{ M, D } = eval_main(TNum, Pid, N),

	% Возвращение кортежей, состоящих из эмпирических и теоретических математического ожидания и дисперсии
	{ { M, 1 / Lambda }, { D, 1 / pow(Lambda, 2) } }.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Главная функция главного вычислительного потока
eval_main(TNum, Pid, N) ->

	% Получение суммы реализация псевдослучайной величины и расчет математического ожидания
	M = get_V(TNum, 0) / N,

	% Отправка математического ожидания всем вычислительным потокам
	foreach(fun(TPid) -> TPid ! { M, mean } end, Pid),

	% Получение суммы квадратов центрированных реализаций псевдослучайной величины, расчет дисперсии, возвращение кортежа, состоящего из математического ожидания и дисперсии
	{ M, get_V(TNum, 0) / N }.

% С помощью функции get_V() главный вычислительный поток суммирует значения, присылаемые ему остальными вычислительными потоками
get_V(0, V) -> V;
get_V(TNum, V) ->

	receive

		{ TM, mean } ->

			get_V(TNum - 1, V + TM);

		{ TD, disp } ->

			get_V(TNum - 1, V + TD)
			
	end.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Главная функция вычислительных потоков
thread_main(PPid, Lambda, N) ->

	% Инициализация датчика случайных чисел
	seed(now()),

	% Генерирование N реализаций псевдослучайной величины с показательным распределением с помощью датчика псевдослучайных чисел, распределенных равномерно на диапазоне [0; 1)
	L = map(fun(_) -> - log(uniform()) / Lambda end, seq(1, N)),

	% Отправка суммы реализаций псевдослучайной величины главному вычислительному потоку
	PPid ! { sum(L), mean },

	TD =
		receive

			% Получение от главного вычислительного потока математического ожидания
			{ M, mean } ->

				% Расчет суммы квадратов центрированных реализаций псевдослучайной величины
				sum(map(fun(X) -> pow(X - M, 2) end, L))

		end,

	% Отправка главному вычислительному потоку суммы квадратов центрированных реализаций псевдослучайной величины
	PPid ! { TD, disp }.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

