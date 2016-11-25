using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AllNetBridgeCS
{
    public class AllNetBoards
    {
        public List<AllNetBoard> AllNetBoardList { get; set; }

        public void AddAllNetBoard(AllNetBoard allNetBoard)
        {
            if (this.AllNetBoardList == null) this.AllNetBoardList = new List<AllNetBoard>();

            if(!AllNetBoardExists(allNetBoard))
            {
                AllNetBoardList.Add(allNetBoard);
            }
        }

        public void UpdateAllNetBoardInListWithAllNetMessage(AllNetBoard allNetBoard, AllNetMessage allNetMessage)
        {
            foreach (var allNetBoardItem in AllNetBoardList)
            {
                if (allNetBoardItem.Id == allNetBoard.Id)
                {
                    allNetBoardItem.UpdateWithMessage(allNetMessage);
                }
            }
        }

        public bool AllNetBoardExists(AllNetBoard allNetBoard)
        {
            if (GetAllNetBoardById(allNetBoard.Id) != null) return true;
            return false; 
        }

        public AllNetBoard GetAllNetBoardById(int Id)
        {
            var query = (from b in this.AllNetBoardList where b.Id == Id select b).FirstOrDefault();
            return query;
        }


    }


}
