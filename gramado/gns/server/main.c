/*
 * File: main.c
 * 
 *    Main file for the Gramado Network Server.
 * 
 *    O propósito é servir 'arquivos' e 'aplicações' para a rede.
 *    Talvez ele chame o servidor de protocolos como helper.
 *
 *    History:
 *        2020 - Created by Fred Nora. 
 */


// main.c
// Arquivo principal do gws.
// As funções começam com o nome do módulo
// para que em client side começem com gws_


/*
See: https://wiki.osdev.org/Graphics_stack

    > Application Layer
    > Interoperation Layers
        +Desktop Management Layer
        +Window Management Layer 
    > Presentation Layers
        +Compositing Layer
        +Widget Toolkit Layer
        +Rendering Layer 
    > Display Layers
        +Device Driver Layer
        +Hardware Layer 

*/

/*	$NetBSD: tty.c,v 1.8 2011/09/06 18:34:57 joerg Exp $	*/

/*
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <sys/cdefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <packet.h>
#include <gns.h>

#include <api.h>

#include <sys/socket.h>



//
// O buffer para  as mensagens recebidas via socket.
//

char __buffer[512];   


int running = 0;


// Our desktop;
struct desktop_d *__desktop;


// Our PID.
//int __ws_pid;
int __ns_pid;   // network server pid.

//window.
struct window_d *__mywindow; 


int ____saved_server_fd = -1;


// Esses valores serão enviados como 
// resposta ao serviço atual.
// Eles são configurados pelo dialogo na hora da 
// prestação do serviço.
// No início desse array fica o header.
unsigned long next_response[32];


void gns_yield(void);
  
  

/*
 * gwsProcedure:
 *     Dialog to handle the event loop.
 * 
 */
 
int 
gnsProcedure ( 
    void *window, 
    int msg, 
    unsigned long long1, 
    unsigned long long2 );



// internal.
// Messages sended via socket.
// obs: read and write use the buffer '__buffer'
// in the top of this file.

void __socket_messages (int fd){

    // Isso permite ler a mensagem na forma de longs.
    unsigned long *message_buffer = (unsigned long *) &__buffer[0];   

    int n_reads = 0;     // For requests.
    int n_writes = 0;    // For responses.


    if (fd<0){
        gde_debug_print ("__socket_messages: fd\n");
        return;
    }


//__loop:

    // #todo
    // Devemos escrever em nosso próprio
    // socket e o kernel copia??
    // o kernel copia para aquele arquivo ao qual esse estivere conectado.
    // olhando em accept[0]

    n_reads = read ( fd, __buffer, sizeof(__buffer));

    // 
    // Se nao tem o que ler. saimos. 
    if (n_reads <= 0)
        return;
        
    // Nesse momento lemos alguma coisa.   
 
    //debug_print ("gws: request found on its own socket \n");  
            
    //  mensagem invalida  
    if (message_buffer[1] == 0 )
        return;


    debug_print ("gns: Got a request!\n");
    debug_print ("gns: Calling window procedure \n");
                
    //#debug: para a máquina real.
    //printf ("gws: got a message!\n");
    //printf ("gws: __socket_messages: calling window procedure \n");
 
                
    // realiza o serviço.
    gnsProcedure ( (void *) message_buffer[0], 
       (int) message_buffer[1], 
       (unsigned long) message_buffer[2], 
       (unsigned long) message_buffer[3] );


    // #todo
    // Se o request foi um request de evento,
    // significa que o cliente deseja receber o próximo evento da 
    // lista de eventos.
    // podemos passar mensagens recebidas pelo gws para o cliente.

    // ??
    // espera ate conseguir enviar a resposta.
    // o kernel precisa copiar para aquele conectado em accept[]

    //
    // Sending reply.
    // 
     
    gde_debug_print ("Sending response ...\n");  

__again:

    // #debug: para a máquina real.
    debug_print ("Sending response ...\n");
                
    // #debug
    //sprintf (__buffer," ................. This is a response");
                
    // Primeiros longs do buffer.
    message_buffer[0] = next_response[0];         //  Window ID.
    message_buffer[1] = SERVER_PACKET_TYPE_REPLY; //next_response[1] 
    message_buffer[2] = next_response[2];         // Return value (long1)
    message_buffer[3] = next_response[3];         // Return value (long2)

    n_writes = write ( fd, __buffer, sizeof(__buffer) );
    if (n_writes<=0)
        goto __again;


    // Cleaning
    message_buffer[0] = 0;
    message_buffer[1] = 0;
    message_buffer[2] = 0;
    message_buffer[3] = 0;

    // Cleaning
    int c=0;
    for(c=0; c<32; c++)
        next_response[c] = 0;


    gde_debug_print ("Response sent\n");  
}



// internal
// System ipc messages. (It's like a signal)
void __ipc_message (void){

    unsigned long message_buffer[5];   

    gde_enter_critical_section ();
    gramado_system_call ( 111,
            (unsigned long) &message_buffer[0],
            (unsigned long) &message_buffer[0],
            (unsigned long) &message_buffer[0] );
    gde_exit_critical_section ();

    //se não tem mensagem
    if ( message_buffer[1] == 0 )
         return;

        
    // Send message to the window procedure.
    gnsProcedure ( (void *) message_buffer[0], 
        (int) message_buffer[1], 
        (unsigned long) message_buffer[2], 
        (unsigned long) message_buffer[3] );

     message_buffer[0] = 0;
     message_buffer[1] = 0;
     message_buffer[2] = 0;
     message_buffer[3] = 0;
}


/*
 **********************************
 * gnsProcedure:
 *     Main dialog.
 * 
 */
 

int 
gnsProcedure ( 
    void *window, 
    int msg, 
    unsigned long long1, 
    unsigned long long2 )
{

    int my_pid = -1;


    debug_print ("gwsProcedure:\n");
  
    switch (msg)
    {
        // Com essa mensagem o cliente solicita input.
        // então devemos drenar input usado loop de mensagens e
        // repassar para o cliente via socket.
        case 8080:
            gde_debug_print ("gns: [TODO] 8080. drain messages ...\n");
            break;

        case MSG_SYSKEYUP:
            switch ( long1)
            {
                // #debug
                case VK_F1:
                    gde_reboot ();
                    break;
                    
                // #debug
                // mostrar informações sobre o socket 
                // do gws.    
                case VK_F2:
                    my_pid = (int) getpid();
                    gramado_system_call (7008,my_pid,0,0);
                    break; 
             
                // Enviar a mensagem para o processo associado
                // com a janela que tem o foco de entrada.
                default:
                    gde_debug_print ("gns: MSG_SYSKEYUP\n");
                    break;
            }    
            break;

        // #test
        // Tentando receber uma mensagem de digitação.
        case MSG_KEYDOWN:
            
            //#bugbug: Isso está falhando.
            //mas o f1 funciona.
            //printf ("%c", (char) long1); 
            //gws_show_backbuffer ();
            
            gde_debug_print ("gns: MSG_KEYDOWN\n");
            break;

        case 1000:
            printf (">> Hello from Gramado Network Server !\n");
            break;

        case 1001:
            break; 
            
            
        case 1002:
            break;

               
        case 1003:
            break;
               
             
        case 2000:
            break;
 
        case 2001:
            break;
             
             
        case 2002:
            break;
             
             
        case 2003:
            break;


        // Refresh screen 
        // refresh screen using kgws service. 
        case 2020:
             //gws_show_backbuffer ();
             break;
             

        // Refresh rectangle ... 
        case 2021:
            break;

        // ...
             
                
        default:
            //printf ("msg=%d ",msg);
            break;
    }


    // #todo: 
    // Call the system's window procedure.    
    // Rever esse retorno.
    //return (int) gde_system_procedure (window,msg,long1,long2);
    return 0;
}



/*
 ******************************
 * main: 
 *     gns main routine.
 */

int main (int argc, char **argv){

    unsigned long w=0;
    unsigned long h=0;


    // Isso registra uma gramado port.
    // a porta do ws.
    // mas isso ja foi feito na rotina que registra o ws.
    // nunca testado.
    //gramado_system_call (7006, 11, getpid(), 0 );

    struct sockaddr addr;
    addr.sa_family = AF_GRAMADO;
    addr.sa_data[0] = 'n';
    addr.sa_data[1] = 's';   
    
    
    int server_fd = -1; 
    int bind_status = -1;
    
    int i=0;


    // Flag usada no loop.
    running = 1;
    

    // Serial debug.
    gde_debug_print ("--------------------------\n");
    gde_debug_print ("gns: Initializing ...\n");

    // #debug
    printf ("gns: gns is alive !  \n");

    
    // Init gws infrastructure.
    
    //gwsInit ();

    
    // #tests
    // Isso funciona.
    //pixelBackBufferPutpixel ( COLOR_RED,   100, 250 );
    //pixelBackBufferPutpixel ( COLOR_GREEN, 105, 250 );
    //pixelBackBufferPutpixel ( COLOR_BLUE,  110, 250 );
    //charBackbufferDrawcharTransparent ( 250,       250, COLOR_RED,   (unsigned long) 'R');
    //charBackbufferDrawcharTransparent ( 250 +8,    250, COLOR_GREEN, (unsigned long) 'G');
    //charBackbufferDrawcharTransparent ( 250 +8 +8, 250, COLOR_BLUE,  (unsigned long) 'B');
    //charBackbufferDrawchar ( 300, 300, (unsigned long) 'X', COLOR_YELLOW, COLOR_RED );
    //lineBackbufferDrawHorizontalLine ( 400, 88, 500, COLOR_PINK );
    //rectBackbufferDrawRectangle ( 200, 400, 100, 60, COLOR_YELLOW );


    //createwCreateWindow ( WT_SIMPLE, 1, 1, "FIRST-WINDOW",  
        //10, 60, 
        //(800/3), (600/3),   
        //gui->screen, 0, xCOLOR_GRAY3, xCOLOR_GRAY3 );
    

    //createwCreateWindow ( WT_EDITBOX, 1, 1, "FIRST-WINDOW",  
      //  80, 80, 
      //  300, 40,   
      // gui->screen, 0, xCOLOR_GRAY3, xCOLOR_GRAY3 );


    //
    // Task bar
    //

    //w = gws_get_device_width();
    //h = gws_get_device_height();

    
    //if(w==0 && h==0)
        //fail!
   
      /*  
    // #log: isso funcionou na máquina real.    
    __mywindow = (struct window_d *) createwCreateWindow ( WT_SIMPLE, 
                                         1, 1, "gws-taskbar",  
                                         0, 0, w, 36,   
                                         gui->screen, 0, 
                                         COLOR_GRAY, COLOR_GRAY );

     */
 

    //
    // Desktop
    //

    // Getting current desktop;

    __desktop = (struct desktop_d *) gramado_system_call (519,0,0,0);


     //draw text inside a window.
     //draw_text ( (struct window_d *) __mywindow,
       // 40, 40,
        //COLOR_RED,
         //"Drawing some text inside a window!" );


    //
    // Register.
    //

    // Set ws PID
    // Setar esse processo como o ws do sistema.
    //__ws_pid = (int) getpid();
    //gramado_system_call ( 513, __desktop, __ws_pid, __ws_pid );
    
    __ns_pid = (int) getpid();
    gramado_system_call ( 521, __desktop, __ns_pid, __ns_pid );


    //
    // socket
    //
    
    // #debug
    printf ("gns: Creating socket\n");

    server_fd = (int) socket (AF_GRAMADO, SOCK_STREAM, 0);
    
    if (server_fd<0){
        printf("gns: Couldn't create the server socket\n");
        exit(1);
    }
 
    ____saved_server_fd = server_fd;

 
    //
    // bind
    // 
 
    // #debug
    printf ("gns: bind\n");
 
    bind_status = bind ( server_fd, (struct sockaddr *) &addr, sizeof(addr) );

    if (bind_status<0){
        printf("gns: Couldn't bind to the socket\n");
        exit(1);
    }


    //
    // =======================================
    //


    //
    // Calling child.
    //

    //dtextDrawText ( (struct window_d *) gui->screen,
        //8, 80, COLOR_RED, "gws: Calling child" );
        
    
    printf ("Hello from gns\n");
    
    // #atenção: 
    //na máquina real, isso mostrou a barra, 
    //mas não mostrou a string criada na janela gui->screen..
    //gws_show_backbuffer ();




    printf ("gns: * Calling child \n");
    // #test
    // Nesse test, s2 usará socket para se conectar
    // AF_GRAMADO.


    gde_clone_and_execute ("gnst.bin");  //client.  
    //gde_clone_and_execute ("??.bin");  
    //gde_clone_and_execute ("??.bin");  
    // ...

 
    printf ("gns: * yield \n");

    for (i=0; i<11; i++)
        gns_yield ();


    //
    // =======================================
    //

// loop:

     while (1){

        // Messages sended via sockets.
        // Mensagens enviadas pelos clientes via socket.
        // Inclusive a mensagem que pede para drenar input e 
        // via mensagens e repassar via socket. (8080)
        __socket_messages (____saved_server_fd);

        //...
    };


    //
    // =======================================
    //

    // Done.
    
    gde_debug_print ("gns: exited. \n");
    printf ("gns: exited. \n");
    
    return 0; 
}


void gns_yield(void){
	
    gramado_system_call (265,0,0,0); //yield thread.
}








