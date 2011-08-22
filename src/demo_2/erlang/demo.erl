
-module(demo).
-author(akinin_mv).
-import(global, [register_name / 2, sync / 0, send / 2]).
-import(lists, [split / 2, map / 2, seq / 2, foreach / 2, foldl / 3]).
-export([prun/5, eval/4]).

prun(PNum, Num, MFrom, MTo, Fun) ->

	register_name(parent, self()),

	Step = (MTo - MFrom) / PNum,
	SmallStep = Step / Num,
	{LFrom_1, LFrom_2} = split(PNum div 2, map(fun(U) -> MFrom + Step * U end, seq(0, PNum - 1))),

	foreach(fun(From) -> spawn('amv_2@127.0.0.1', demo_erlang, eval, [From, SmallStep, Num, Fun]) end, LFrom_1),
	foreach(fun(From) -> spawn('amv_3@127.0.0.1', demo_erlang, eval, [From, SmallStep, Num, Fun]) end, LFrom_2),

	mrs(PNum, 0).

mrs(0, Sum) ->

	Sum;

mrs(PNum, Sum) ->

	receive

		{ R } ->

			mrs(PNum - 1, Sum + R)

	end.

eval(From, Step, Num, Fun) ->

	sync(),
	send(parent, { Step * foldl(fun(X, Sum) -> Sum + Fun(From + Step * X) end, 0, seq(0, Num - 1)) }).

