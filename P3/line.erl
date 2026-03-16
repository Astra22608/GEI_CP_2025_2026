-module(line).

-export([start/1, send/2, bounce/3, stop/1]).

start(N) ->
    Pids = crear_procesos(N, 0),
    conectar(Pids, nil),
    hd(Pids).

%% Función auxiliar para spawnear procesos
crear_procesos(0, _) -> [];
crear_procesos(N, Id) ->
    [spawn(fun() -> loop(Id, nil, nil) end) | crear_procesos(N-1, Id+1)].

%% Configura a cada proceso su vecino anterior y siguiente
conectar([], _) -> ok;
conectar([Pid], Prev) -> Pid ! {set_links, Prev, nil};
conectar([Pid, Next | Rest], Prev) ->
    Pid ! {set_links, Prev, Next},
    conectar([Next | Rest], Pid).

send(Pid, Msg) ->
    Pid ! {send, Msg},
    ok.

bounce(Pid, Msg, Times) ->
    Pid ! {bounce, Msg, Times, forward},
    ok.

stop(Pid) ->
    Pid ! stop,
    ok.

loop(Id, Prev, Next) ->
    receive
        {set_links, P, N} -> 
            loop(Id, P, N);
        {send, Msg} ->
            io:format("~p received message ~p~n", [Id, Msg]),
            if Next /= nil -> Next ! {send, Msg}; true -> ok end,
            loop(Id, Prev, Next);
        {bounce, Msg, Times, Dir} ->
            io:format("~p received message ~p~n", [Id, Msg]),
            if Times > 1 ->
                if 
                    Dir == forward, Next /= nil -> Next ! {bounce, Msg, Times-1, forward};
                    Dir == forward, Next == nil -> Prev ! {bounce, Msg, Times-1, backward};
                    Dir == backward, Prev /= nil -> Prev ! {bounce, Msg, Times-1, backward};
                    Dir == backward, Prev == nil -> Next ! {bounce, Msg, Times-1, forward}
                end;
                true -> ok
            end,
            loop(Id, Prev, Next);
        stop ->
            if Next /= nil -> Next ! stop; true -> ok end
    end.