/*
** Scicos_SerialPort.c
**
** Made by Quentin Quadrat
** Mail   <lecrapouille@gmail.com>
**
** Started on  Thu Sep 28 16:23:12 2006 Quentin Quadrat
** Last update Thu Feb  3 02:18:09 2011 Quentin Quadrat
*/

#include <assert.h>
#include "Scicos_SerialPort.h"

/*****************************************************************************************/
/* Static Variables                                                                      */
/*****************************************************************************************/

/** \brief Physical Layer name (i.g. device /dev/ttyS0 for Unix systems). The name comes
 * from the Scilab interface function (*.sci) */
static char           portname[64];

/** \brief Array of Data Link Layer */
static struct s_DataLinkLayer   *DataLinkLayers[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
#define CURRENT_SERIALPORT       DataLinkLayers[block->ipar[E_NTH_SERIALPORT]]

static uint8                     DataLinkLayer_errors[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#define CURRENT_SP_ERRORS        DataLinkLayer_errors[block->ipar[0]]

/** \brief Bootloader */
static struct s_tftp             tftp;

/** \brief Store informations */
//static struct s_BlackBox      BlackBox;

/* conversion int -> flaot */
#define SCICOS_SERIAL_PORT_BUFFER_SIZE          255
static uint16 *msg16b;


// ENUMERATOR for the 'scicos_block->ipar' array
#define E_SPEED               0
#define E_DATABITS            1
#define E_PARITY              2
#define E_STOPBITS            3
#define E_FLOWCONTROL         4
#define E_NTH_SERIALPORT      5
#define E_BUFFER_READ         5
#define E_BUFFER_WRITE        5
#define E_SIZE_NAME           6
#define E_BEGIN_NAME          7
#define E_END_NAME            ipar[E_SIZE_NAME + E_BEGIN_NAME]

/*****************************************************************************************/
/** \brief Alloc memory for the Physical Layer and the Data Link Layer. Call their
 * initialisations.
 *
 * @param portname
 *      The Physical Layer name (device).
 *
 * @param id_sp
 *      (not used)
 *
 * @param baud
 *      Strategy of Physical Layer baud rate.
 *
 * @param nbbits
 *      Strategy concerning the number of bits that forms a data.
 *
 * @param nbstop
 *      Strategy concerning the number of stop bits to transmit after the data bits.
 *
 * @param parity
 *      Strategy concerning the parity check of the data.
 *
 * @param flow
 *      Strategy concerning the control of flow.
 *
 * @param signal_handler_IO
 *      For asynchrone mode: function pointer to a handler function which is triggered when
 *      data are presents in the Physical Layer input. If do not want asynchrone mode set
 *      this parameter to NULL.
 *
 * @return
 *      - The address of the allocated/initialised Data Link Layer structure.
 *      - NULL if an error occured.
 */
/*****************************************************************************************/
static struct s_DataLinkLayer      *serialport_new(const char    /* IN/OUT */   *const portname,
                                                  const int32   /* IN */              sp_id,
                                                  const int32   /* IN */              baud,
                                                  const int32   /* IN */              nbbits,
                                                  const int32   /* IN */              nbstop,
                                                  const int32   /* IN */              parity,
                                                  const int32   /* IN */              flow,
                                                  void          /* FUNC */ (*signal_handler_IO) (int))
{
  struct s_UART          *PhysicalLayer;

  /* ------------------------------------------------------------------ */
  /* Physical Layer initialisation                                      */
  /* ------------------------------------------------------------------ */

  if ((PhysicalLayer = malloc(sizeof (struct s_UART))) == NULL)
    {
      fprintf(stderr, "Creating Serial Port: Cannot allocate memory for the file descriptor.\n");
      return NULL;
    }

  if (UART_open(PhysicalLayer, portname, baud, nbbits, nbstop, parity, flow) == FAILED)
    {
      fprintf(stderr, "Creating Serial Port: Cannot open the device '%s'.\n", portname);
      //free(PhysicalLayer);
      return NULL;
    }

  /* ------------------------------------------------------------------ */
  /* Data Link Layer initialisation                                     */
  /* ------------------------------------------------------------------ */
  if ((DataLinkLayers[sp_id] = malloc(sizeof (struct s_DataLinkLayer))) == NULL)
    {
      fprintf(stderr, "Creating Serial Port: Cannot allocate memory for the data link layer.\n");
      //UART_close(PhysicalLayer);
      //free(PhysicalLayer);
      return NULL;
    }

  DataLinkLayers[sp_id]->asynchrone = FALSE;
  if (signal_handler_IO != NULL)
    {
      /* Attach a handler function to the Physical Layer in the aim to get
       * an interrupt when data are present */
      if (UART_set_asynchrone(PhysicalLayer, signal_handler_IO) == FAILED)
        {
          fprintf(stderr, "Creating Serial Port: Cannot set asynchrone the Serial Port.\n");
          //UART_close(PhysicalLayer);
          //free(PhysicalLayer);
          //free(DataLinkLayers[sp_id]);
          return NULL;
        }
      DataLinkLayers[sp_id]->asynchrone = TRUE;
    }

  /* Physical Layer Accessors */
  DataLinkLayers[sp_id]->read  = (pf_read) UART_read8b;
  DataLinkLayers[sp_id]->write = (pf_write) UART_write8b;
  DataLinkLayers[sp_id]->close = (pf_close) UART_close;
  DataLinkLayers[sp_id]->tx_not_full = (pf_txnotfull) UART_is_tx_empty;

  DataLinkLayer_init(DataLinkLayers[sp_id], PhysicalLayer);
  return DataLinkLayers[sp_id];
}

/*****************************************************************************************/
/** \brief Alloc memory for the Physical Layer and the Data Link Layer. Call their
 * initialisations.
 *
 * @param block
 *      The Scicos bloc to determine which serial port has to be created.
 *
 * @return 0 if no error. -1 if an error occured.
 */
/*****************************************************************************************/
static uint8            scicos_open_serialports(scicos_block       /* IN/OUT */  *const block)//,
//struct s_BlackBox  /* IN/OUT */  *const BlackBox)
{
  int32                 job = 1;
  int32                 i;
  int32                 status;

  fprintf(stderr, "****************************************************\n");
  //for (i = 0; i < block->nipar; ++i)
  i = 0;
  {
    /* Test if the current serial port is in used */
    if (CURRENT_SERIALPORT != NULL)
      {
        fprintf(stderr, "Serial Port already opened.\n");
        //BlackBox_add_error(BlackBox, SCICOS_SP_ALREADY_OPENED, &block->ipar[E_NTH_SERIALPORT], 1);
        status = FAILED;
      }
    else
      {
        /* Get the name of the device from the interface function (the *.sci Scilab function) */
        F2C(cvstr)(&(block->ipar[E_SIZE_NAME]), &(block->ipar[E_BEGIN_NAME]), portname, &job, strlen(portname));
        portname[block->ipar[E_SIZE_NAME]] = '\0';

        /* Open a new serial port */
        CURRENT_SERIALPORT = serialport_new(portname,                      /* device name (i.g. "/dev/ttyS0") */
                                            block->ipar[E_NTH_SERIALPORT], /* for CURRENT_SERIALPORT */
                                            block->ipar[E_SPEED],          /* Baud rate (i.g. 115200) */
                                            block->ipar[E_DATABITS],       /* number of data bits (i.g. 8) */
                                            block->ipar[E_STOPBITS],       /* number of stop bits (i.g. 1) */
                                            block->ipar[E_PARITY],         /* Bit of parity strategy (i.g. 0) */
                                            block->ipar[E_FLOWCONTROL],    /* Flow control strategy (i.g. none) */
                                            NULL);                         /* Rx handler function */
        if (CURRENT_SERIALPORT == NULL)
          {
            fprintf(stderr, "ID %d: Initialisation of the serial port failed !!\n", block->ipar[E_NTH_SERIALPORT]);
            //BlackBox_add_error(BlackBox, SCICOS_SP_INIT_FAILURE, &block->ipar[E_NTH_SERIALPORT], 1);
            status = FAILED;
          }
        else
          {
            fprintf(stderr, "ID %d: Initialisation of the serial succeeded !!\n", block->ipar[E_NTH_SERIALPORT]);
            //BlackBox_add_error(BlackBox, SCICOS_SP_OPENED, &block->ipar[E_NTH_SERIALPORT], 1);
            status = SUCCEEDED;
          }
      }
  }

  fprintf(stderr, "****************************************************\n\n");
  return status;
}

/*****************************************************************************************/
/** \brief Destroy/Free the Data Link Layer memory.
 *
 * @param block
 *      The Scicos bloc to determine which serial port has to be destroyed.
 */
/*****************************************************************************************/
static void               scicos_close_serialports(scicos_block  *block)
{
  int32 i;

  fprintf(stderr, "****************************************************\n");
  //for (i = 0; i < block->nipar; ++i)
  i = 0;
    {
      /* Test if already destroyed */
      if (CURRENT_SERIALPORT == NULL)
        {
          fprintf(stderr, "Serial Port Scicos Bloc: already destroyed !\n");
        }
      else
        {
          /* Destroying */
          DataLinkLayer_close(CURRENT_SERIALPORT);
          free(CURRENT_SERIALPORT->physical_layer);
          free(CURRENT_SERIALPORT);
          CURRENT_SERIALPORT = NULL;
          fprintf(stderr, "Serial Port Scicos Bloc %d: closed !\n", block->ipar[E_NTH_SERIALPORT]);
        }
    }
  fprintf(stderr, "****************************************************\n\n");
}

/*****************************************************************************************/
/** \biref */
/*****************************************************************************************/
void                    serialport_scicos_printf(scicos_block *block, int flag)
{
  int32                 job = 1;

  switch (flag)
    {
    case SCICOS_INIT_PHASE:
    case SCICOS_END_PHASE:
      break;

    case SCICOS_LOOP_PHASE:
      F2C(cvstr)(&(block->ipar[0]), &(block->ipar[1]), portname, &job, strlen(portname));
      portname[block->ipar[0]] = '\0';
      fprintf(stderr, portname);
      break;
    }
}

/*****************************************************************************************/
/** \biref */
/*****************************************************************************************/
void                    serialport_scicos_bootloader(scicos_block *block, int flag)
{
  switch (flag)
    {
      /* ------------------------------------------------------------------ */
    case SCICOS_INIT_PHASE:
      // assert(1 == 2);
      //      assert(CURRENT_SERIALPORT == NULL);
      // TFTP_Server_init(&tftp, CURRENT_SERIALPORT/*, &BlackBox*/);
      /* Init Status */
      block->outptr[0][0] = IN_PROGRESS;
      break;
      /* ------------------------------------------------------------------ */
    case SCICOS_END_PHASE:
      TFTP_Server_close(&tftp);
      break;
      /* ------------------------------------------------------------------ */
    case SCICOS_LOOP_PHASE:
      /* Do nothing if the serial port has been badly created */
      if (DataLinkLayers[block->ipar[0]] != NULL)
      {
        /* TFTP Server putfile */
        block->outptr[0][0] = /* Status */
          TFTP_Server_update(&tftp);
      }
      break;
    default: /* Do nothing */ break;
    }
}

/*****************************************************************************************/
/** \biref Scicos function: Write in the Serial Port
 *
 * @param block
 *      The Scicos block (managed by Scicos).
 *
 * @param flag
 *      The Scicos state (managed by Scicos):
 *      SCICOS_INIT_PHASE, SCICOS_LOOP_PHASE, SCICOS_END_PHASE, ...
 */
/*****************************************************************************************/
void                     serialport_scicos_write(scicos_block *block, int flag)
{
  int32                  i;
  uint8                  status, length;

  switch (flag)
    {
      /* ------------------------------------------------------------------ */
    case SCICOS_INIT_PHASE:
      /* Init Status */
      block->inptr[1][0] = IN_PROGRESS;
      break;
      /* ------------------------------------------------------------------ */
    case SCICOS_END_PHASE:
      /* Do nothing */
      block->inptr[1][0] = IN_PROGRESS;
      break;
      /* ------------------------------------------------------------------ */
    case SCICOS_LOOP_PHASE:
      /* Do nothing if the serial port has been badly created */
      if (DataLinkLayers[block->ipar[0]] != NULL)
        {
          fprintf(stderr, "<== Sending message:'");
          i = 0;
          status = SUCCEEDED;
          while ((i < block->ipar[E_BUFFER_WRITE]) && (status == SUCCEEDED))
            {
              msg16b[i] = block->inptr[0][i];
              fprintf(stderr, " %u", msg16b[i]);
              if (block->inptr[0][i] > (float32) 255)
                {
                  // BlackBox_add_error(&BlackBox, AL_MSG_DROPPED, (uint8*) &block->inptr[0][i], 4*2);
                  fprintf(stderr, "...'\n<== Error: %f is > 255\n", block->inptr[0][i]);
                  fprintf(stderr, "This message will not be send.");
                  status = FAILED;
                }
              ++i;
            }
          if (status == SUCCEEDED)
            {
              fprintf(stderr, "'\n");
              status = DataLinkLayer_Send16bMessage(CURRENT_SERIALPORT, msg16b, length/*, BlackBox*/);
            }
          block->inptr[1][0] = status;
        }
      break;
      /* ------------------------------------------------------------------ */
    default: /* Do nothing */ break;
    }
}

/*****************************************************************************************/
/** \biref */
/*****************************************************************************************/
void                     serialport_scicos_read(scicos_block /* IN/OUT */ *block,
                                                int          /* IN */      flag)
{
  uint8                  status, length, i;

  switch (flag)
    {
      /* ------------------------------------------------------------------ */
    case SCICOS_INIT_PHASE:
      /* Init Status */
      block->inptr[1][0] = IN_PROGRESS;
      //CURRENT_SP_ERRORS = 0;
      break;
      /* ------------------------------------------------------------------ */
    case SCICOS_END_PHASE:
      /* Do nothing */
      block->inptr[1][0] = IN_PROGRESS;
      break;
      /* ------------------------------------------------------------------ */
    case SCICOS_LOOP_PHASE:
      /* Do nothing if the serial port has been badly created */
      if (DataLinkLayers[block->ipar[0]] != NULL)
        {
          //if (CURRENT_SP_ERRORS == 10)
            {
              /* FAILURE */
            }
          status = DataLinkLayer_ReadMessage(CURRENT_SERIALPORT, &msg16b, &length);
          switch (status)
            {
            case FAILED:
              /* keep previous message */
              ++CURRENT_SP_ERRORS;
              break;
            case SUCCEEDED:
              if (length != block->ipar[E_BUFFER_READ])
                {
                  fprintf(stderr, "ERR scicos: bad length\n");
                  ++CURRENT_SP_ERRORS;
                  status = FAILED;
                }
              else
                {
                  //CURRENT_SP_ERRORS = 0;
                  for (i = 0; i < length; ++i)
                    {
                      block->outptr[0][i] = msg16b[i];
                      fprintf(stderr, " %u", msg16b[i]);
                    }
                }
              break;
            default:
              fprintf(stderr, "ERR scicos: read bad staus\n");
              //++CURRENT_SP_ERRORS;
              status = FAILED;
              break;
            }
          block->inptr[1][0] = status;
        }
      break;
      /* ------------------------------------------------------------------ */
    default: /* Do nothing */ break;
    }
}

/*****************************************************************************************/
/** \biref Scicos function: Read in the Serial Port
 *
 * @param block
 *      The Scicos block (managed by Scicos).
 *
 * @param flag
 *      The Scicos state (managed by Scicos):
 *      SCICOS_INIT_PHASE, SCICOS_LOOP_PHASE, SCICOS_END_PHASE, ...
 */
/*****************************************************************************************/
void                    serialport_scicos_init(scicos_block *block, int flag)
{
  //struct s_clock        clck;

  switch (flag)
    {
      /* ------------------------------------------------------------------ */
    case SCICOS_INIT_PHASE:
      fprintf(stderr, "INIT\n");
      assert(BSC_BUFFER_SIZE == 64);
      /* Black Box initialisation */
      //gettimeofday(&clck, NULL);
      //BlackBox_init(&BlackBox, &clck);
      //BlackBox_add_error(&BlackBox, H4H_IS_RUNNING, NULL, 0);
      /* Create a new serial port */
      block->outptr[0][0] = scicos_open_serialports(block/*, &BlackBox*/) ;
      if ((block->outptr[0][0] == SUCCEEDED) && (CURRENT_SERIALPORT != NULL))
        {
          TFTP_Server_init(&tftp, CURRENT_SERIALPORT/*, &BlackBox*/);
        }
      else
        {
          // fprintf(stderr, "STAUS = %f 0x%x\n", block->outptr[0][0], CURRENT_SERIALPORT);
          scicos_close_serialports(block);
        }
      break;
      /* ------------------------------------------------------------------ */
    case SCICOS_END_PHASE:
      //scicos_close_serialports(block);
      //BlackBox_display_errors(&BlackBox, stderr);
      break;
      /* ------------------------------------------------------------------ */
    case SCICOS_LOOP_PHASE:
      /* Do nothing */
      break;
    default: /* Do nothing */ break;
    }
}
